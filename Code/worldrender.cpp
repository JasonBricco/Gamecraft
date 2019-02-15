//
// Jason Bricco
//

// Builds mesh data for the chunk.
static void BuildChunk(World* world, Chunk* chunk)
{
    BEGIN_TIMED_BLOCK(BUILD_CHUNK);

    for (int z = 0; z < CHUNK_SIZE_X; z++)
    {
        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int x = 0; x < CHUNK_SIZE_X; x++)
            {
                Block block = GetBlock(chunk, x, y, z);

                if (block != BLOCK_AIR)
                {
                    BlockMeshType type = GetMeshType(world, block);
                    Mesh* mesh = chunk->meshes[type];

                    if (mesh == nullptr)
                    {
                        mesh = CreateMesh(8192, 12288, MESH_NO_FLAGS);
                        chunk->meshes[type] = mesh;
                    }

                    BuildFunc(world, block)(world, chunk, mesh, x, y, z, block);
                }
            }
        }
    }

    chunk->state = CHUNK_NEEDS_FILL;

    END_TIMED_BLOCK(BUILD_CHUNK);
}

static void FillChunkMeshes(Chunk* chunk)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
        Mesh* mesh = chunk->meshes[i];

        if (mesh != nullptr)
        {
            assert(mesh->vertCount > 0);
            FillMeshData(mesh, GL_DYNAMIC_DRAW);
        }
    }
}

static void BuildChunkNow(World* world, Chunk* chunk)
{
    BuildChunk(world, chunk);
    FillChunkMeshes(chunk);
    chunk->state = CHUNK_BUILT;
    chunk->pendingUpdate = false;
}

static bool NeighborsHaveState(World* world, Chunk* chunk, ChunkState state)
{
    LChunkPos p = chunk->lcPos;

    for (int i = 0; i < 8; i++)
    {
        LChunkPos next = p + DIRECTIONS_2D[i];
        Chunk* c = GetChunkSafe(world, next);

        if (c == nullptr || c->state < state)
            return false;
    }

    return true;
}

static void ProcessVisibleChunks(GameState* state, World* world, Camera* cam, Chunk** visibleChunks, int visibleCount)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
        ChunkMeshList& list = cam->meshLists[i];
        list.meshes = AllocTempArray(visibleCount, ChunkMesh);
        list.count = 0;
    }

    vec2 playerChunk = vec2(world->loadRange, world->loadRange);

    sort(visibleChunks, visibleChunks + visibleCount, [playerChunk](auto a, auto b) 
    { 
        float distA = distance2(vec2(a->lcPos.x, a->lcPos.z), playerChunk);
        float distB = distance2(vec2(b->lcPos.x, b->lcPos.z), playerChunk);
        return distA < distB;
    });

    for (int i = 0; i < visibleCount; i++)
    {
        Chunk* chunk = visibleChunks[i];
        assert(chunk->active);

        switch (chunk->state)
        {
            case CHUNK_LOADED:
            {
                if (NeighborsHaveState(world, chunk, CHUNK_LOADED))
                {
                    chunk->state = CHUNK_BUILDING;
                    world->buildCount++;
                    QueueAsync(state, BuildChunk, world, chunk);
                }
            } break;

            case CHUNK_NEEDS_FILL:
                FillChunkMeshes(chunk);
                chunk->state = CHUNK_BUILT;
                world->buildCount--;
                break;

            case CHUNK_BUILT:
            {
                if (chunk->pendingUpdate)
                    BuildChunkNow(world, chunk);

                for (int m = 0; m < CHUNK_MESH_COUNT; m++)
                {
                    Mesh* mesh = chunk->meshes[m];

                    if (mesh != nullptr)
                    {
                        ChunkMesh cM = { mesh, (vec3)chunk->lwPos };
                        ChunkMeshList& list = cam->meshLists[m];
                        list.meshes[list.count++] = cM;
                    }
                }
            } break;
        }
    }
}

static Chunk** GetVisibleChunks(World* world, Camera* cam, int& visibleCount)
{    
    Chunk** visibleChunks = AllocTempArray(world->totalChunks, Chunk*);

    for (int i = 0; i < world->totalChunks; i++)
    {
        Chunk* chunk = world->chunks[i];

        // Ensure chunks that need to be filled are considered visible so that
        // their meshes will be filled. This prevents blocking the world generation
        // when the building chunks can't decrement.
        if (chunk->state == CHUNK_NEEDS_FILL)
        {
            visibleChunks[visibleCount++] = chunk;
            continue;
        }
        
        ivec3 cP = chunk->lcPos * CHUNK_SIZE_X;
        vec3 min = vec3(cP.x, 0.0f, cP.z);
        vec3 max = min + (vec3(CHUNK_SIZE_X, WORLD_HEIGHT, CHUNK_SIZE_X) - 1.0f);

        FrustumVisibility visibility = TestFrustum(cam, min, max);

        if (visibility >= FRUSTUM_VISIBLE)
            visibleChunks[visibleCount++] = chunk;
    }

    return visibleChunks;
}

// Returns true if the current block should draw its face when placed
// next to the adjacent block.
static inline bool CanDrawFace(World* world, CullType cur, Block adjBlock)
{
    CullType adj = GetCullType(world, adjBlock);

    if (cur == CULL_OPAQUE)
        return adj >= CULL_TRANSPARENT;
    else return adj == CULL_ALL;
}

#define BLOCK_TRANSPARENT(pos) GetCullType(world, GetBlockSafe(world, chunk, pos)) >= CULL_TRANSPARENT

static inline Colori VertexLight(World* world, Chunk* chunk, Axis axis, RelPos pos, int dx, int dy, int dz)
{
    RelPos a, b, c, d;

    switch (axis)
    {
        case AXIS_X:
            a = pos + ivec3(dx, 0, 0);
            b = pos + ivec3(dx, dy, 0);
            c = pos + ivec3(dx, 0, dz);
            d = pos + ivec3(dx, dy, dz);
            break;

        case AXIS_Y:
            a = pos + ivec3(0, dy, 0);
            b = pos + ivec3(dx, dy, 0);
            c = pos + ivec3(0, dy, dz);
            d = pos + ivec3(dx, dy, dz);
            break;

        default:
            a = pos + ivec3(0, 0, dz);
            b = pos + ivec3(dx, 0, dz);
            c = pos + ivec3(0, dy, dz);
            d = pos + ivec3(dx, dy, dz);
            break;
    }

    bool t1 = BLOCK_TRANSPARENT(b);
    bool t2 = BLOCK_TRANSPARENT(c);

    if (t1 || t2) 
    {
        Colori c1 = BLOCK_TRANSPARENT(a) ? NewColori(255) : NewColori(65);
        Colori c2 = BLOCK_TRANSPARENT(b) ? NewColori(255) : NewColori(65);
        Colori c3 = BLOCK_TRANSPARENT(c) ? NewColori(255) : NewColori(65);
        Colori c4 = BLOCK_TRANSPARENT(d) ? NewColori(255) : NewColori(65);

        return AverageColor(c1, c2, c3, c4);
    }
    else 
    {
        Colori c1 = BLOCK_TRANSPARENT(a) ? NewColori(255) : NewColori(65);
        Colori c2 = BLOCK_TRANSPARENT(b) ? NewColori(255) : NewColori(65);
        Colori c3 = BLOCK_TRANSPARENT(c) ? NewColori(255) : NewColori(65);

        return AverageColor(c1, c2, c3);
    }
}

#define LIGHT(a, o1, o2, o3) VertexLight(world, chunk, AXIS_##a, rP, o1, o2, o3)

static inline void SetFaceVertexData(Mesh* mesh, int index, float x, float y, float z, Colori c)
{
    mesh->positionData[index] = vec3(x, y, z);
    mesh->colorData[index] = c;
}

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
static void BuildBlock(World* world, Chunk* chunk, Mesh* mesh, int xi, int yi, int zi, Block block)
{
    float* textures = GetTextures(world, block);

    RelPos rP = ivec3(xi, yi, zi);
    float x = (float)xi, y = (float)yi, z = (float)zi;

    CullType cull = GetCullType(world, block);

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi + 1, zi)))
    {
        int count = mesh->vertCount;

        CheckMeshBounds(mesh, 4, count, mesh->vertMax);
        SetIndices(mesh);
        SetUVs(mesh, textures[FACE_TOP]);

        SetFaceVertexData(mesh, count, x + 0.5f, y + 0.5f, z - 0.5f, LIGHT(Y, 1, 1, -1));
        SetFaceVertexData(mesh, count + 1, x + 0.5f, y + 0.5f, z + 0.5f, LIGHT(Y, 1, 1, 1));
        SetFaceVertexData(mesh, count + 2, x - 0.5f, y + 0.5f, z + 0.5f, LIGHT(Y, -1, 1, 1));
        SetFaceVertexData(mesh, count + 3, x - 0.5f, y + 0.5f, z - 0.5f, LIGHT(Y, -1, 1, -1));

        mesh->vertCount += 4;
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi - 1, zi)))
    {
        int count = mesh->vertCount;

        CheckMeshBounds(mesh, 4, count, mesh->vertMax);
        SetIndices(mesh);
        SetUVs(mesh, textures[FACE_BOTTOM]);

        SetFaceVertexData(mesh, count, x - 0.5f, y - 0.5f, z - 0.5f, LIGHT(Y, -1, -1, -1));
        SetFaceVertexData(mesh, count + 1, x - 0.5f, y - 0.5f, z + 0.5f, LIGHT(Y, -1, -1, 1));
        SetFaceVertexData(mesh, count + 2, x + 0.5f, y - 0.5f, z + 0.5f, LIGHT(Y, 1, -1, 1));
        SetFaceVertexData(mesh, count + 3, x + 0.5f, y - 0.5f, z - 0.5f, LIGHT(Y, 1, -1, -1));

        mesh->vertCount += 4;
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi, zi + 1)))
    {
        int count = mesh->vertCount;

        CheckMeshBounds(mesh, 4, count, mesh->vertMax);
        SetIndices(mesh);
        SetUVs(mesh, textures[FACE_FRONT]);

        SetFaceVertexData(mesh, count, x - 0.5f, y - 0.5f, z + 0.5f, LIGHT(Z, -1, -1, 1)); 
        SetFaceVertexData(mesh, count + 1, x - 0.5f, y + 0.5f, z + 0.5f, LIGHT(Z, -1, 1, 1));
        SetFaceVertexData(mesh, count + 2, x + 0.5f, y + 0.5f, z + 0.5f, LIGHT(Z, 1, 1, 1));
        SetFaceVertexData(mesh, count + 3, x + 0.5f, y - 0.5f, z + 0.5f, LIGHT(Z, 1, -1, 1));

        mesh->vertCount += 4;
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi, zi - 1)))
    {
        int count = mesh->vertCount;

        CheckMeshBounds(mesh, 4, count, mesh->vertMax);
        SetIndices(mesh);
        SetUVs(mesh, textures[FACE_BACK]);

        SetFaceVertexData(mesh, count, x + 0.5f, y - 0.5f, z - 0.5f, LIGHT(Z, 1, -1, -1));
        SetFaceVertexData(mesh, count + 1, x + 0.5f, y + 0.5f, z - 0.5f, LIGHT(Z, 1, 1, -1));
        SetFaceVertexData(mesh, count + 2, x - 0.5f, y + 0.5f, z - 0.5f, LIGHT(Z, -1, 1, -1));
        SetFaceVertexData(mesh, count + 3, x - 0.5f, y - 0.5f, z - 0.5f, LIGHT(Z, -1, -1, -1)); 

        mesh->vertCount += 4;
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi + 1, yi, zi)))
    {
        int count = mesh->vertCount;

        CheckMeshBounds(mesh, 4, count, mesh->vertMax);
        SetIndices(mesh);
        SetUVs(mesh, textures[FACE_RIGHT]);

        SetFaceVertexData(mesh, count, x + 0.5f, y - 0.5f, z + 0.5f, LIGHT(X, 1, -1, 1));
        SetFaceVertexData(mesh, count + 1, x + 0.5f, y + 0.5f, z + 0.5f, LIGHT(X, 1, 1, 1));
        SetFaceVertexData(mesh, count + 2, x + 0.5f, y + 0.5f, z - 0.5f, LIGHT(X, 1, 1, -1));
        SetFaceVertexData(mesh, count + 3, x + 0.5f, y - 0.5f, z - 0.5f, LIGHT(X, 1, -1, -1));

        mesh->vertCount += 4;
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi - 1, yi, zi)))
    {
        int count = mesh->vertCount;

        CheckMeshBounds(mesh, 4, count, mesh->vertMax);
        SetIndices(mesh);
        SetUVs(mesh, textures[FACE_LEFT]);

        SetFaceVertexData(mesh, count, x - 0.5f, y - 0.5f, z - 0.5f, LIGHT(X, -1, -1, -1));
        SetFaceVertexData(mesh, count + 1, x - 0.5f, y + 0.5f, z - 0.5f, LIGHT(X, -1, 1, -1));
        SetFaceVertexData(mesh, count + 2, x - 0.5f, y + 0.5f, z + 0.5f, LIGHT(X, -1, 1, 1));
        SetFaceVertexData(mesh, count + 3, x - 0.5f, y - 0.5f, z + 0.5f, LIGHT(X, -1, -1, 1));

        mesh->vertCount += 4;
    }
}
