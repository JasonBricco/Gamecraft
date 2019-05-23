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

static inline void SetChunkMeshData(Renderer& rend, Chunk* chunk)
{
    assert(chunk->state != CHUNK_BUILDING);

    for (int i = 0; i < MESH_TYPE_COUNT; i++)
    {
        assert(chunk->meshData[i] == nullptr);
        chunk->meshData[i] = GetMeshData(rend.meshData);
        assert(chunk->meshData[i] != nullptr);
    }
}

static void BuildChunk(GameState* state, World* world, Chunk* chunk)
{
    Renderer& rend = state->renderer;
    SetChunkMeshData(rend, chunk);

    world->workCount++;
    chunk->state = CHUNK_BUILDING;

    QueueAsync(state, BuildChunkAsync, world, chunk, OnChunkBuilt);
}

static void RebuildChunks(GameState* state, World* world)
{
    Renderer& rend = state->renderer;
    auto& chunks = world->chunksToRebuild;

    for (int i = 0; i < chunks.size(); i++)
        SetChunkMeshData(rend, chunks[i]);

    world->chunksRebuilding = true;
    world->workCount++;
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
        LChunkP next = p + DIRS_2[i];

        ChunkGroup* adj = GetGroup(world, next.x, next.z);
        assert(adj != nullptr);

        if (adj->state == GROUP_DEFAULT || adj->state == GROUP_PREPROCESSING)
            return false;
    }

    return true;
}

static void PreprocessGroup(GameState*, World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;

    Queue<ivec3> sunNodes(MAX_LIGHT_NODES);
    Queue<ivec3> lightNodes(MAX_LIGHT_NODES);
    SetLightNodes(world, group, sunNodes, lightNodes);

    group->state = GROUP_PREPROCESSED;
}

static void OnGroupPreprocessed(GameState*, World* world, void*)
{
    world->workCount--;
}

static void PrepareWorldRender(GameState* state, World* world, Renderer& rend)
{
    TIMED_BLOCK;
    
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
                if (!world->chunksRebuilding && chunk->pendingUpdate)
                {
                    world->chunksToRebuild.push_back(chunk);
                    chunk->pendingUpdate = false;
                }

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
            LChunkP next = lcP + DIRS_2[i];

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

static inline Colori AverageColor(Colori first, Colori second, Colori third, Colori fourth)
{
    return ((ivec4)first + (ivec4)second + (ivec4)third + (ivec4)fourth) >> 2;
}

static inline Colori AverageColor(Colori first, Colori second, Colori third)
{
    return ((ivec4)first + (ivec4)second + (ivec4)third) / 3;
}

static const uint8_t lightOutput[] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };
        
static inline Colori GetFinalLight(Chunk* chunk, int rX, int rY, int rZ)
{
    if (chunk == nullptr)
        return rY == -1 ? Colori(0) : Colori(255);

    int index = BlockIndex(rX, rY, rZ);

    uint8_t light = lightOutput[chunk->blockLight[index]];
    uint8_t sun = lightOutput[GetSunlight(chunk, rX, RelToLWorldP(chunk, rY), rZ, index)];

    return Colori(light, light, light, sun);
}

static inline Colori VertexLight(World* world, Chunk* chunk, Axis axis, RelP pos, int dx, int dy, int dz)
{
    RelP rA, rB, rC, rD;

    switch (axis)
    {
        case AXIS_X:
            rA = pos + ivec3(dx, 0, 0);
            rB = pos + ivec3(dx, dy, 0);
            rC = pos + ivec3(dx, 0, dz);
            rD = pos + ivec3(dx, dy, dz);
            break;

        case AXIS_Y:
            rA = pos + ivec3(0, dy, 0);
            rB = pos + ivec3(dx, dy, 0);
            rC = pos + ivec3(0, dy, dz);
            rD = pos + ivec3(dx, dy, dz);
            break;

        default:
            rA = pos + ivec3(0, 0, dz);
            rB = pos + ivec3(dx, 0, dz);
            rC = pos + ivec3(0, dy, dz);
            rD = pos + ivec3(dx, dy, dz);
            break;
    }

    RebasedPos a = Rebase(world, chunk->lcPos, rA);
    RebasedPos b = Rebase(world, chunk->lcPos, rB);
    RebasedPos c = Rebase(world, chunk->lcPos, rC);

    bool t1 = !IsOpaque(world, GetBlockSafe(b));
    bool t2 = !IsOpaque(world, GetBlockSafe(c));

    if (t1 || t2) 
    {
        RebasedPos d = Rebase(world, chunk->lcPos, rD);

        Colori c1 = GetFinalLight(a.chunk, a.rX, a.rY, a.rZ);
        Colori c2 = GetFinalLight(b.chunk, b.rX, b.rY, b.rZ);
        Colori c3 = GetFinalLight(c.chunk, c.rX, c.rY, c.rZ);
        Colori c4 = GetFinalLight(d.chunk, d.rX, d.rY, d.rZ);

        return AverageColor(c1, c2, c3, c4);
    }
    else 
    {
        Colori c1 = GetFinalLight(a.chunk, a.rX, a.rY, a.rZ);
        Colori c2 = GetFinalLight(b.chunk, b.rX, b.rY, b.rZ);
        Colori c3 = GetFinalLight(c.chunk, c.rX, c.rY, c.rZ);

        return AverageColor(c1, c2, c3);
    }
}

#define LIGHT(a, o1, o2, o3) VertexLight(world, chunk, AXIS_##a, rP, o1, o2, o3)

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
