//
// Gamecraft
//

// Builds mesh data for the chunk.
static void BuildChunkAsync(GameState*, World* world, void* chunkPtr)
{
    Chunk* chunk = (Chunk*)chunkPtr;
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

                    BuildFunc(world, block)(world, chunk, data, x, y, z, block);
                }
            }
        }
    }
}

static void RebuildChunksAsync(GameState* state, World* world, void* chunksPtr)
{
    auto& chunks = *(vector<Chunk*>*)chunksPtr;

    for (int i = 0; i < chunks.size(); i++)
        BuildChunkAsync(state, world, chunks[i]);
}

static void OnChunkBuilt(GameState*, World* world, void* chunkPtr)
{
    Chunk* chunk = (Chunk*)chunkPtr;
    world->workCount--;
    assert(world->workCount >= 0);
    chunk->state = CHUNK_NEEDS_FILL;
}

static void OnChunksRebuilt(GameState*, World* world, void* chunksPtr)
{
    auto& chunks = *(vector<Chunk*>*)chunksPtr;

    for (int i = 0; i < chunks.size(); i++)
        chunks[i]->state = CHUNK_NEEDS_FILL;

    chunks.clear();

    world->chunksRebuilding = false;

    world->workCount--;
    assert(world->workCount >= 0);
}

static inline bool SetChunkMeshData(Renderer& rend, Chunk* chunk)
{
    assert(chunk->state != CHUNK_BUILDING);

    for (int i = 0; i < MESH_TYPE_COUNT; i++)
    {
        assert(chunk->meshData[i] == nullptr);
        chunk->meshData[i] = GetMeshData(rend.meshData);
    }

    return true;
}

static void BuildChunk(GameState* state, World* world, Chunk* chunk)
{
    Renderer& rend = state->renderer;

    if (!rend.meshData.CanGet(MESH_TYPE_COUNT))
        return;

    SetChunkMeshData(rend, chunk);

    world->workCount++;
    chunk->state = CHUNK_BUILDING;

    QueueAsync(state, BuildChunkAsync, world, chunk, OnChunkBuilt);
}

static void RebuildChunks(GameState* state, World* world)
{
    Renderer& rend = state->renderer;
    auto& chunks = world->chunksToRebuild;

    if (!rend.meshData.CanGet(MESH_TYPE_COUNT * (int)chunks.size()))
        return;

    for (int i = 0; i < chunks.size(); i++)
    {
        chunks[i]->pendingUpdate = false;
        SetChunkMeshData(rend, chunks[i]);
    }

    world->workCount++;
    world->chunksRebuilding = true;

    QueueAsync(state, RebuildChunksAsync, world, &world->chunksToRebuild, OnChunksRebuilt);
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
        assert(data != nullptr);

        // The vertex count would be 0 if the only blocks belonging to the mesh type were culled away. 
        if (data->vertCount > 0)
        {
            FillMeshData(rend.meshData, chunk->meshes[i], data, GL_DYNAMIC_DRAW, 0);
            chunk->hasMeshes = true;
        }
        else rend.meshData.Return(data);

        chunk->meshData[i] = nullptr;
    }
}

static void DestroyChunkMeshes(Chunk* chunk)
{
    for (int i = 0; i < MESH_TYPE_COUNT; i++)
        DestroyMesh(chunk->meshes[i]);
}

static bool AllowPreprocess(World* world, ChunkGroup* group)
{
    LChunkP p = group->chunks->lcPos;

    for (int i = 0; i < 9; i++)
    {
        LChunkP next = p + DIRS[i];

        ChunkGroup* adj = GetGroup(world, next.x, next.z);
        assert(adj != nullptr);

        if (adj->state == CHUNK_DEFAULT || adj->state == GROUP_PREPROCESSING)
            return false;
    }

    return true;
}

static void PreprocessGroup(GameState*, World*, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;
    group->state = GROUP_PREPROCESSED;
}

static void OnGroupPreprocessed(GameState*, World* world, void*)
{
    world->workCount--;
}

static void PrepareWorldRender(GameState* state, World* world, Renderer& rend)
{
    for (int i = 0; i < MESH_TYPE_COUNT; i++)
        rend.meshLists[i].clear();

    auto& visible = world->visibleChunks;

    for (int i = 0; i < visible.size(); i++)
    {
        Chunk* chunk = visible[i];

        switch (chunk->state)
        {
            case CHUNK_DEFAULT:
            case CHUNK_LOADED_DATA:
                BuildChunk(state, world, chunk);
                break;

            case CHUNK_NEEDS_FILL:
            {
                if (chunk->pendingUpdate)
                    ReturnChunkMeshes(rend, chunk);
                else FillChunkMeshes(rend, chunk);

                chunk->state = CHUNK_BUILT;
            }

            case CHUNK_BUILT:
            {
                if (chunk->pendingUpdate)
                    world->chunksToRebuild.push_back(chunk);

                for (int m = 0; m < MESH_TYPE_COUNT; m++)
                {
                    Mesh mesh = chunk->meshes[m];

                    if (mesh.indexCount > 0)
                    {
                        ChunkMesh cM = { mesh, (vec3)chunk->lwPos };
                        vector<ChunkMesh>& list = rend.meshLists[m];
                        list.push_back(cM);
                    }
                }
            } break;
        }
    }

    if (!world->chunksRebuilding && world->chunksToRebuild.size() > 0)
        RebuildChunks(state, world);

    Biome& biome = GetCurrentBiome(world);
    rend.emitters.push_back(biome.weather.emitter);
}

static void WorldRenderUpdate(GameState* state, World* world, Camera* cam)
{    
    world->visibleChunks.clear();
    world->groupsToProcess.clear();

    for (int g = 0; g < world->totalGroups; g++)
    {
        ChunkGroup* group = world->groups[g];

        if (group->pendingDestroy || IsEdgeGroup(world, group)) 
            continue;

        world->groupsToProcess.push_back(group);
    }

    vec2 playerChunk = vec2(world->loadRange, world->loadRange);

    auto& groups = world->groupsToProcess;
    sort(groups.begin(), groups.end(), [playerChunk](auto a, auto b)
    {
        float distA = distance2(vec2(a->chunks->lcPos.x, a->chunks->lcPos.z), playerChunk);
        float distB = distance2(vec2(b->chunks->lcPos.x, b->chunks->lcPos.z), playerChunk);
        return distA < distB;
    });
        
    for (int g = 0; g < world->groupsToProcess.size(); g++)
    {
        ChunkGroup* group = world->groupsToProcess[g];

        if (group->state == GROUP_LOADED && AllowPreprocess(world, group))
        {
            group->state = GROUP_PREPROCESSING;
            world->workCount++;
            QueueAsync(state, PreprocessGroup, world, group, OnGroupPreprocessed);
        }

        LChunkP lcP = group->chunks->lcPos;
        bool allowVisible = true;

        for (int i = 0; i < 9; i++)
        {
            LChunkP next = lcP + DIRS[i];

            ChunkGroup* adj = GetGroup(world, next.x, next.z);
            assert(adj != nullptr);

            if (adj->state != GROUP_PREPROCESSED)
            {
                allowVisible = false;
                break;
            }
        }

        if (allowVisible)
        {
            for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
            {
                Chunk* chunk = group->chunks + i;

                if (chunk->state == CHUNK_NEEDS_FILL)
                {
                    world->visibleChunks.push_back(chunk);
                    continue;
                }
                
                ivec3 cP = LChunkToLWorldP(chunk->lcPos);
                vec3 min = vec3(cP.x, cP.y, cP.z);
                vec3 max = min + (vec3(CHUNK_SIZE_H, CHUNK_SIZE_V, CHUNK_SIZE_H) - 1.0f);

                FrustumVisibility visibility = TestFrustum(cam, min, max);

                if (visibility >= FRUSTUM_VISIBLE)
                    world->visibleChunks.push_back(chunk);
            }
        }
    }
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
        u8vec3 c2 = t1 ? u8vec3(255) : u8vec3(65);
        u8vec3 c3 = t2 ? u8vec3(255) : u8vec3(65);
        u8vec3 c4 = BLOCK_TRANSPARENT(d) ? u8vec3(255) : u8vec3(65);

        return AverageColor(c1, c2, c3, c4);
    }
    else 
    {
        u8vec3 c1 = BLOCK_TRANSPARENT(a) ? u8vec3(255) : u8vec3(65);
        u8vec3 c2 = t1 ? u8vec3(255) : u8vec3(65);
        u8vec3 c3 = t2 ? u8vec3(255) : u8vec3(65);

        return AverageColor(c1, c2, c3);
    }
}

#define LIGHT(a, o1, o2, o3) NewColori(VertexLight(world, chunk, AXIS_##a, rP, o1, o2, o3), alpha)

static inline void SetFaceVertexData(MeshData* data, int index, float x, float y, float z, Colori c)
{
    data->positions[index] = vec3(x, y, z);
    data->colors[index] = c;
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

static inline bool CheckFace(World* world, CullType cull, Block block, RebasedPos p, int& vAdded)
{
    if (CanDrawFace(world, cull, block, GetBlockSafe(p)))
    {
        vAdded += 4;
        return true;
    }

    return false;
}

// Returns true if an assertion should be triggered due to overflow and 
// false otherwise. If the chunk has a build mask already, we do not want 
// to assert on overflow as we'll handle this case separately.
static inline bool DebugOverflowCheck(Chunk* chunk, int vAdded)
{
    return chunk->totalVertices + vAdded > MAX_VERTICES;
}

static inline bool ChunkOverflowed(World* world, Chunk* chunk, int x, int y, int z)
{
    int index = BlockIndex(x, y, z);
    Block block = chunk->blocks[index];

    if (block != BLOCK_AIR)
    {
        CullType cull = GetCullType(world, block);

        // The number of vertices we'll add by building this block. If it exceeds the 
        // chunk limit, we'll return immediately.
        int vAdded = 0;

        NeighborBlocks adj = GetNeighborBlocks(world, chunk, x, y, z);
        
        CheckFace(world, cull, block, adj.up, vAdded);
        CheckFace(world, cull, block, adj.down, vAdded);
        CheckFace(world, cull, block, adj.front, vAdded);
        CheckFace(world, cull, block, adj.back, vAdded);
        CheckFace(world, cull, block, adj.right, vAdded);
        CheckFace(world, cull, block, adj.left, vAdded);

        if (chunk->totalVertices + vAdded > MAX_VERTICES)
            return true;
    }

    return false;
}

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block)
{
    uint16_t* textures = GetTextures(world, block);

    ivec3 rP = ivec3(xi, yi, zi);
    float x = (float)xi, y = (float)yi, z = (float)zi;

    uint8_t alpha = GetBlockAlpha(world, block);
    CullType cull = GetCullType(world, block);

    int vAdded = 0;
    NeighborBlocks adj = GetNeighborBlocks(world, chunk, xi, yi, zi);

    if (CheckFace(world, cull, block, adj.up, vAdded))
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

    if (CheckFace(world, cull, block, adj.down, vAdded))
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

    if (CheckFace(world, cull, block, adj.front, vAdded))
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

    if (CheckFace(world, cull, block, adj.back, vAdded))
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

    if (CheckFace(world, cull, block, adj.right, vAdded))
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

    if (CheckFace(world, cull, block, adj.left, vAdded))
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

    assert(!DebugOverflowCheck(chunk, vAdded));
    chunk->totalVertices += vAdded;
}
