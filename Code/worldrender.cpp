//
// Gamecraft
//

// Builds mesh data for the chunk.
static void BuildChunk(GameState* state, World* world, void* chunkPtr)
{
    Renderer& rend = state->renderer;

    Chunk* chunk = (Chunk*)chunkPtr;
    assert(!IsEdgeChunk(world, chunk));
    chunk->totalVertices = 0;

    for (int y = 0; y < CHUNK_SIZE_V; y++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int x = 0; x < CHUNK_SIZE_H; x++)
            {
                Block block = GetBlock(chunk, x, y, z);

                if (block != BLOCK_AIR)
                {
                    BlockMeshType type = GetMeshType(world, block);
                    MeshData* data = chunk->meshData[type];

                    while (data == nullptr)
                    {
                        EnterCriticalSection(&rend.meshCS);
                        data = GetMeshData(rend.meshData);

                        if (data == nullptr)
                            SleepConditionVariableCS(&rend.meshDataEmpty, &rend.meshCS, 16);
                        else 
                        {
                            assert(data->vertCount == 0);
                            assert(data->indexCount == 0);
                            chunk->meshData[type] = data;
                        }

                        LeaveCriticalSection(&rend.meshCS);
                    }

                    if (!BuildFunc(world, block)(world, chunk, data, x, y, z, block))
                        chunk->overflowCount++;
                }
            }
        }
    }

    chunk->state = CHUNK_NEEDS_FILL;
}

static void ReturnChunkMeshes(Renderer& rend, Chunk* chunk)
{
    for (int i = 0; i < MESH_TYPE_COUNT; i++)
    {
        MeshData* data = chunk->meshData[i];

        if (data != nullptr)
        {
            rend.meshData.Return(data);
            chunk->meshData[i] = nullptr;
        }
    }
}

static void FillChunkMeshes(Renderer& rend, Chunk* chunk)
{
    if (chunk->hasMeshes)
    {
        DestroyChunkMeshes(chunk);
        chunk->hasMeshes = false;
    }

    for (int i = 0; i < MESH_TYPE_COUNT; i++)
    {
        MeshData* data = chunk->meshData[i];

        if (data == nullptr) continue;

        // The vertex count would be 0 if the only blocks belonging to the mesh type were culled away. 
        if (data->vertCount > 0)
        {
            FillMeshData(rend.meshData, chunk->meshes[i], data, GL_DYNAMIC_DRAW, 0);
            chunk->hasMeshes = true;
        }
        else rend.meshData.Return(data);

        chunk->meshData[i] = nullptr;
    }

    WakeAllConditionVariable(&rend.meshDataEmpty);
}

static void DestroyChunkMeshes(Chunk* chunk)
{
    for (int i = 0; i < MESH_TYPE_COUNT; i++)
        DestroyMesh(chunk->meshes[i]);
}

static void RebuildChunk(GameState* state, World* world, Chunk* chunk)
{
    chunk->pendingUpdate = false;
    world->buildCount++;
    QueueAsync(state, BuildChunk, world, chunk);
}

static bool NeighborsLoaded(World* world, Chunk* chunk)
{
    LChunkP p = chunk->lcPos;

    for (int i = 0; i < 8; i++)
    {
        LChunkP next = p + DIRS_2D[i];
        ChunkGroup* group = GetGroupSafe(world, next.x, next.z);

        if (group == nullptr || !group->loaded)
            return false;
    }

    return true;
}

static void RemoveOverflowAndRebuild(World* world, Chunk* chunk)
{
    for (int y = CHUNK_SIZE_V - 1; y >= 0; y--)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int x = 0; x < CHUNK_SIZE_H; x++)
            {
                if (GetBlock(chunk, x, y, z) != BLOCK_AIR)
                {
                    SetBlock(chunk, x, y, z, BLOCK_AIR);
                    FlagChunkForUpdate(world, chunk, chunk->lcPos, ivec3(x, y, z), false);

                    if (--chunk->overflowCount == 0)
                        return;
                }
            }
        }
    }
}

static void ProcessVisibleChunks(GameState* state, World* world, Renderer& rend)
{
    for (int i = 0; i < MESH_TYPE_COUNT; i++)
    {
        ChunkMeshList& list = rend.meshLists[i];
        list.meshes = AllocTempArray(world->visibleChunks.size, ChunkMesh);
        list.count = 0;
    }

    vec2 playerChunk = vec2(world->loadRange, world->loadRange);

    List<Chunk*> visible = world->visibleChunks;
    sort(visible.items, visible.items + visible.size, [playerChunk](auto a, auto b)
    {
        float distA = distance2(vec2(a->lcPos.x, a->lcPos.z), playerChunk);
        float distB = distance2(vec2(b->lcPos.x, b->lcPos.z), playerChunk);
        return distA < distB;
    });

    for (int i = 0; i < visible.size; i++)
    {
        Chunk* chunk = visible[i];

        switch (chunk->state)
        {
            case CHUNK_DEFAULT:
            case CHUNK_LOADED_DATA:
            {
                if (NeighborsLoaded(world, chunk))
                {
                    chunk->state = CHUNK_BUILDING;
                    world->buildCount++;
                    QueueAsync(state, BuildChunk, world, chunk);
                }
            } break;

            case CHUNK_NEEDS_FILL:
            {
                if (chunk->pendingUpdate)
                    ReturnChunkMeshes(rend, chunk);
                else FillChunkMeshes(rend, chunk);

                chunk->state = CHUNK_BUILT;
                world->buildCount--;

                if (chunk->overflowCount > 0)
                    RemoveOverflowAndRebuild(world, chunk);
            }

            case CHUNK_BUILT:
            case CHUNK_REBUILDING:
            {
                if (chunk->state != CHUNK_REBUILDING && chunk->pendingUpdate)
                {
                    chunk->state = CHUNK_REBUILDING;
                    RebuildChunk(state, world, chunk);
                }

                for (int m = 0; m < MESH_TYPE_COUNT; m++)
                {
                    Mesh mesh = chunk->meshes[m];

                    if (mesh.indexCount > 0)
                    {
                        ChunkMesh cM = { mesh, (vec3)chunk->lwPos };
                        ChunkMeshList& list = rend.meshLists[m];
                        list.meshes[list.count++] = cM;
                    }
                }
            } break;
        }
    }
}

static void GetVisibleChunks(World* world, Camera* cam)
{    
    world->visibleChunks.Clear();

    for (int g = 0; g < world->totalGroups; g++)
    {
        ChunkGroup* group = world->groups[g];

        if (!group->loaded || group->pendingDestroy) 
            continue;

        if (IsEdgeGroup(world, group))
            continue;

        for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
        {
            Chunk* chunk = group->chunks + i;

            // Ensure chunks that need to be filled are considered visible so that
            // their meshes will be filled. This prevents blocking when a visible chunk
            // begins building but is no longer visible at the time of needing to be filled.
            if (chunk->state == CHUNK_NEEDS_FILL)
            {
                world->visibleChunks.AddLast(chunk);
                continue;
            }
            
            ivec3 cP = LChunkToLWorldP(chunk->lcPos);
            vec3 min = vec3(cP.x, cP.y, cP.z);
            vec3 max = min + (vec3(CHUNK_SIZE_H, CHUNK_SIZE_V, CHUNK_SIZE_H) - 1.0f);

            FrustumVisibility visibility = TestFrustum(cam, min, max);

            if (visibility >= FRUSTUM_VISIBLE)
                world->visibleChunks.AddLast(chunk);
        }
    }
}

// Returns true if the current block should draw its face when placed
// next to the adjacent block.
static inline bool CanDrawFace(World* world, CullType cur, Block block, Block adjBlock)
{
    CullType adj = GetCullType(world, adjBlock);

    switch (cur)
    {
        case CULL_OPAQUE:
            return adj >= CULL_TRANSPARENT;
        
        case CULL_TRANSPARENT:
            return adj >= CULL_TRANSPARENT && adjBlock != block;
    }

    return false;
}

static inline u8vec3 AverageColor(u8vec3 first, u8vec3 second, u8vec3 third, u8vec3 fourth)
{
    int r = (first.r + second.r + third.r + fourth.r) >> 2;
    int b = (first.b + second.b + third.b + fourth.b) >> 2;
    int g = (first.g + second.g + third.g + fourth.g) >> 2;
    
    return u8vec3(r, g, b);
}

static inline u8vec3 AverageColor(u8vec3 first, u8vec3 second, u8vec3 third)
{
    int r = (first.r + second.r + third.r) / 3;
    int b = (first.b + second.b + third.b) / 3;
    int g = (first.g + second.g + third.g) / 3;
    
    return u8vec3(r, b, g);
}

#define BLOCK_TRANSPARENT(pos) GetCullType(world, GetBlockSafe(world, chunk, pos)) >= CULL_TRANSPARENT

static inline ivec3 VertexLight(World* world, Chunk* chunk, Axis axis, RelP pos, int dx, int dy, int dz)
{
    RelP a, b, c, d;

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
        u8vec3 c1 = BLOCK_TRANSPARENT(a) ? u8vec3(255) : u8vec3(65);
        u8vec3 c2 = BLOCK_TRANSPARENT(b) ? u8vec3(255) : u8vec3(65);
        u8vec3 c3 = BLOCK_TRANSPARENT(c) ? u8vec3(255) : u8vec3(65);
        u8vec3 c4 = BLOCK_TRANSPARENT(d) ? u8vec3(255) : u8vec3(65);

        return AverageColor(c1, c2, c3, c4);
    }
    else 
    {
        u8vec3 c1 = BLOCK_TRANSPARENT(a) ? u8vec3(255) : u8vec3(65);
        u8vec3 c2 = BLOCK_TRANSPARENT(b) ? u8vec3(255) : u8vec3(65);
        u8vec3 c3 = BLOCK_TRANSPARENT(c) ? u8vec3(255) : u8vec3(65);

        return AverageColor(c1, c2, c3);
    }
}

#define LIGHT(a, o1, o2, o3) NewColori(VertexLight(world, chunk, AXIS_##a, rP, o1, o2, o3), alpha)

static inline void SetFaceVertexData(MeshData* data, int index, float x, float y, float z, Colori c)
{
    data->positions[index] = vec3(x, y, z);
    data->colors[index] = c;
}

static inline bool CheckFace(World* world, Chunk* chunk, CullType cull, Block block, int x, int y, int z, int& vAdded)
{
    if (CanDrawFace(world, cull, block, GetBlockSafe(world, chunk, x, y, z)))
    {
        vAdded += 4;
        return true;
    }

    return false;
}

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
static bool BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block)
{
    uint16_t* textures = GetTextures(world, block);

    RelP rP = ivec3(xi, yi, zi);
    float x = (float)xi, y = (float)yi, z = (float)zi;

    uint8_t alpha = GetBlockAlpha(world, block);
    CullType cull = GetCullType(world, block);

    // The number of vertices we'll add by building this block. If it exceeds the 
    // chunk limit, we'll return immediately.
    int vAdded = 0;
    
    bool up = CheckFace(world, chunk, cull, block, xi, yi + 1, zi, vAdded);
    bool down = CheckFace(world, chunk, cull, block, xi, yi - 1, zi, vAdded);
    bool front = CheckFace(world, chunk, cull, block, xi, yi, zi + 1, vAdded);
    bool back = CheckFace(world, chunk, cull, block, xi, yi, zi - 1, vAdded);
    bool right = CheckFace(world, chunk, cull, block, xi + 1, yi, zi, vAdded);
    bool left = CheckFace(world, chunk, cull, block, xi - 1, yi, zi, vAdded);

    if (chunk->totalVertices + vAdded > MAX_VERTICES)
        return false;

    if (up)
    {
        int count = data->vertCount;

        SetIndices(data);
        SetUVs(data, textures[FACE_TOP]);

        SetFaceVertexData(data, count, x + 0.5f, y + 0.5f, z - 0.5f, LIGHT(Y, 1, 1, -1));
        SetFaceVertexData(data, count + 1, x + 0.5f, y + 0.5f, z + 0.5f, LIGHT(Y, 1, 1, 1));
        SetFaceVertexData(data, count + 2, x - 0.5f, y + 0.5f, z + 0.5f, LIGHT(Y, -1, 1, 1));
        SetFaceVertexData(data, count + 3, x - 0.5f, y + 0.5f, z - 0.5f, LIGHT(Y, -1, 1, -1));

        data->vertCount += 4;
    }

    if (down)
    {
        int count = data->vertCount;

        SetIndices(data);
        SetUVs(data, textures[FACE_BOTTOM]);

        SetFaceVertexData(data, count, x - 0.5f, y - 0.5f, z - 0.5f, LIGHT(Y, -1, -1, -1));
        SetFaceVertexData(data, count + 1, x - 0.5f, y - 0.5f, z + 0.5f, LIGHT(Y, -1, -1, 1));
        SetFaceVertexData(data, count + 2, x + 0.5f, y - 0.5f, z + 0.5f, LIGHT(Y, 1, -1, 1));
        SetFaceVertexData(data, count + 3, x + 0.5f, y - 0.5f, z - 0.5f, LIGHT(Y, 1, -1, -1));

        data->vertCount += 4;
    }

    if (front)
    {
        int count = data->vertCount;

        SetIndices(data);
        SetUVs(data, textures[FACE_FRONT]);

        SetFaceVertexData(data, count, x - 0.5f, y - 0.5f, z + 0.5f, LIGHT(Z, -1, -1, 1)); 
        SetFaceVertexData(data, count + 1, x - 0.5f, y + 0.5f, z + 0.5f, LIGHT(Z, -1, 1, 1));
        SetFaceVertexData(data, count + 2, x + 0.5f, y + 0.5f, z + 0.5f, LIGHT(Z, 1, 1, 1));
        SetFaceVertexData(data, count + 3, x + 0.5f, y - 0.5f, z + 0.5f, LIGHT(Z, 1, -1, 1));

        data->vertCount += 4;
    }

    if (back)
    {
        int count = data->vertCount;

        SetIndices(data);
        SetUVs(data, textures[FACE_BACK]);

        SetFaceVertexData(data, count, x + 0.5f, y - 0.5f, z - 0.5f, LIGHT(Z, 1, -1, -1));
        SetFaceVertexData(data, count + 1, x + 0.5f, y + 0.5f, z - 0.5f, LIGHT(Z, 1, 1, -1));
        SetFaceVertexData(data, count + 2, x - 0.5f, y + 0.5f, z - 0.5f, LIGHT(Z, -1, 1, -1));
        SetFaceVertexData(data, count + 3, x - 0.5f, y - 0.5f, z - 0.5f, LIGHT(Z, -1, -1, -1)); 

        data->vertCount += 4;
    }

    if (right)
    {
        int count = data->vertCount;

        SetIndices(data);
        SetUVs(data, textures[FACE_RIGHT]);

        SetFaceVertexData(data, count, x + 0.5f, y - 0.5f, z + 0.5f, LIGHT(X, 1, -1, 1));
        SetFaceVertexData(data, count + 1, x + 0.5f, y + 0.5f, z + 0.5f, LIGHT(X, 1, 1, 1));
        SetFaceVertexData(data, count + 2, x + 0.5f, y + 0.5f, z - 0.5f, LIGHT(X, 1, 1, -1));
        SetFaceVertexData(data, count + 3, x + 0.5f, y - 0.5f, z - 0.5f, LIGHT(X, 1, -1, -1));

        data->vertCount += 4;
    }

    if (left)
    {
        int count = data->vertCount;

        SetIndices(data);
        SetUVs(data, textures[FACE_LEFT]);

        SetFaceVertexData(data, count, x - 0.5f, y - 0.5f, z - 0.5f, LIGHT(X, -1, -1, -1));
        SetFaceVertexData(data, count + 1, x - 0.5f, y + 0.5f, z - 0.5f, LIGHT(X, -1, 1, -1));
        SetFaceVertexData(data, count + 2, x - 0.5f, y + 0.5f, z + 0.5f, LIGHT(X, -1, 1, 1));
        SetFaceVertexData(data, count + 3, x - 0.5f, y - 0.5f, z + 0.5f, LIGHT(X, -1, -1, 1));

        data->vertCount += 4;
    }

    chunk->totalVertices += vAdded;
    return true;
}
