//
// Jason Bricco
//

static inline MeshData* GetChunkMeshData(World* world)
{
    MeshData* data = world->meshData[--world->meshDataCount];
    data->valid = true;
    return data;
}

static inline void ReturnChunkMeshData(World* world, MeshData* data)
{
    data->vertCount = 0;
    data->indexCount = 0;
    world->meshData[world->meshDataCount++] = data;
}

// Builds mesh data for the chunk.
static void BuildChunk(World* world, Chunk* chunk)
{
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
                    BuildFunc(world, block)(world, chunk, chunk->meshData[type], x, y, z, block);
                }
            }
        }
    }

    chunk->state = CHUNK_NEEDS_FILL;
}

static inline void FillChunkMeshes(World* world, Chunk* chunk)
{
    VertexSpec spec = { true, 3, true, 3, true, 4 };

    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
        MeshData* data = chunk->meshData[i];
    
        if (data->valid && data->vertCount > 0)
            FillMeshData(chunk->meshes[i], data, GL_DYNAMIC_DRAW, spec);

        ReturnChunkMeshData(world, data);
    }
}

static inline void BuildChunkNow(World* world, Chunk* chunk)
{
    BuildChunk(world, chunk);
    FillChunkMeshes(world, chunk);
    chunk->state = CHUNK_BUILT;
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

static void ProcessVisibleChunks(GameState* state, World* world, Camera* cam)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
        ChunkMeshList& list = cam->meshLists[i];
        list.meshes = PushTempArray(world->visibleCount, ChunkMesh);
        list.count = 0;
    }

    for (int i = 0; i < world->visibleCount; i++)
    {
        Chunk* chunk = world->visibleChunks[i];
        assert(chunk->active);

        switch (chunk->state)
        {
            case CHUNK_LOADED:
            {
                if (NeighborsHaveState(world, chunk, CHUNK_LOADED))
                {
                    if (world->meshDataCount < CHUNK_MESH_COUNT)
                        continue;

                    for (int c = 0; c < CHUNK_MESH_COUNT; c++)
                        chunk->meshData[c] = GetChunkMeshData(world);

                    chunk->state = CHUNK_BUILDING;
                    world->buildCount++;
                    QueueAsync(state, BuildChunk, world, chunk);
                }
            } break;

            case CHUNK_NEEDS_FILL:
            {
                FillChunkMeshes(world, chunk);
                chunk->state = CHUNK_BUILT;
                world->buildCount--;
            }

            case CHUNK_BUILT:
            {
                if (chunk->pendingUpdate)
                {
                    BuildChunkNow(world, chunk);
                    chunk->pendingUpdate = false;
                }

                for (int m = 0; m < CHUNK_MESH_COUNT; m++)
                {
                    Mesh mesh = chunk->meshes[m];

                    if (mesh.vertCount > 0)
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

static void GetVisibleChunks(World* world, Camera* cam)
{    
    for (int i = 0; i < world->totalChunks; i++)
    {
        Chunk* chunk = world->chunks[i];

        // Ensure chunks that need to be filled are considered visible so that
        // their meshes will be filled. This prevents blocking the world generation
        // when the building chunks can't decrement.
        if (chunk->state == CHUNK_NEEDS_FILL)
        {
            world->visibleChunks[world->visibleCount++] = chunk;
            continue;
        }
        
        ivec3 cP = chunk->lcPos * CHUNK_SIZE_X;
        vec3 min = vec3(cP.x, 0.0f, cP.z);
        vec3 max = min + (vec3(CHUNK_SIZE_X, WORLD_HEIGHT, CHUNK_SIZE_X) - 1.0f);

        FrustumVisibility visibility = TestFrustum(cam, min, max);

        if (visibility >= FRUSTUM_VISIBLE)
            world->visibleChunks[world->visibleCount++] = chunk;
    }
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

static inline Color VertexLight(World* world, Chunk* chunk, Axis axis, RelPos pos, int dx, int dy, int dz)
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
        Color c1 = BLOCK_TRANSPARENT(a) ? vec4(1.0f) : vec4(0.25f);
        Color c2 = BLOCK_TRANSPARENT(b) ? vec4(1.0f) : vec4(0.25f);
        Color c3 = BLOCK_TRANSPARENT(c) ? vec4(1.0f) : vec4(0.25f);
        Color c4 = BLOCK_TRANSPARENT(d) ? vec4(1.0f) : vec4(0.25f);

        return Average(c1, c2, c3, c4);
    }
    else 
    {
        Color c1 = BLOCK_TRANSPARENT(a) ? vec4(1.0f) : vec4(0.25f);
        Color c2 = BLOCK_TRANSPARENT(b) ? vec4(1.0f) : vec4(0.25f);
        Color c3 = BLOCK_TRANSPARENT(c) ? vec4(1.0f) : vec4(0.25f);

        return Average(c1, c2, c3);
    }
}

#define LIGHT(a, o1, o2, o3) VertexLight(world, chunk, AXIS_##a, rP, o1, o2, o3)

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block)
{
    float* textures = GetTextures(world, block);

    RelPos rP = ivec3(xi, yi, zi);
    float x = (float)xi, y = (float)yi, z = (float)zi;

    CullType cull = GetCullType(world, block);

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi + 1, zi)))
    {
        float tex = textures[FACE_TOP];
        SetMeshIndices(data, 10);
        SetMeshVertex(data, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, LIGHT(Y, 1, 1, -1));
        SetMeshVertex(data, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, LIGHT(Y, 1, 1, 1));
        SetMeshVertex(data, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, LIGHT(Y, -1, 1, 1));
        SetMeshVertex(data, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, LIGHT(Y, -1, 1, -1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi - 1, zi)))
    {
        float tex = textures[FACE_BOTTOM];
        SetMeshIndices(data, 10);
        SetMeshVertex(data, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, LIGHT(Y, -1, -1, -1));
        SetMeshVertex(data, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, LIGHT(Y, -1, -1, 1));
        SetMeshVertex(data, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, LIGHT(Y, 1, -1, 1));
        SetMeshVertex(data, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, LIGHT(Y, 1, -1, -1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi, zi + 1)))
    {
        float tex = textures[FACE_FRONT];
        SetMeshIndices(data, 10);
        SetMeshVertex(data, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, LIGHT(Z, -1, -1, 1)); 
        SetMeshVertex(data, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, LIGHT(Z, -1, 1, 1));
        SetMeshVertex(data, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, LIGHT(Z, 1, 1, 1));
        SetMeshVertex(data, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, LIGHT(Z, 1, -1, 1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi, zi - 1)))
    {
        float tex = textures[FACE_BACK];
        SetMeshIndices(data, 10);
        SetMeshVertex(data, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, LIGHT(Z, 1, -1, -1));
        SetMeshVertex(data, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, LIGHT(Z, 1, 1, -1));
        SetMeshVertex(data, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, LIGHT(Z, -1, 1, -1));
        SetMeshVertex(data, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, LIGHT(Z, -1, -1, -1)); 
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi + 1, yi, zi)))
    {
        float tex = textures[FACE_RIGHT];
        SetMeshIndices(data, 10);
        SetMeshVertex(data, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, LIGHT(X, 1, -1, 1));
        SetMeshVertex(data, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, LIGHT(X, 1, 1, 1));
        SetMeshVertex(data, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, LIGHT(X, 1, 1, -1));
        SetMeshVertex(data, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, LIGHT(X, 1, -1, -1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi - 1, yi, zi)))
    {
        float tex = textures[FACE_LEFT];
        SetMeshIndices(data, 10);
        SetMeshVertex(data, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, LIGHT(X, -1, -1, -1));
        SetMeshVertex(data, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, LIGHT(X, -1, 1, -1));
        SetMeshVertex(data, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, LIGHT(X, -1, 1, 1));
        SetMeshVertex(data, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, LIGHT(X, -1, -1, 1));
    }
}
