//
// Jason Bricco
//

// Returns the y-axis height of the first opaque block scanning from the sky downward.
static int GetOpaqueSurface(World* world, Chunk* chunk, int x, int z)
{
    for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--)
    {
        Block block = GetBlock(chunk, x, y, z);
        
        if (!IsTransparent(world, block))
            return y;
    }
    
    return 0;
}

static inline void ComputeRayAtPosition(World* world, Chunk* chunk, int x, int z)
{
    int surface = GetOpaqueSurface(world, chunk, x, z);
    chunk->rays[z * CHUNK_SIZE_X + x] = (uint8_t)(surface + 1);
}

static void ComputeRays(World* world, Chunk* chunk) 
{
    for (int z = 0; z < CHUNK_SIZE_X; z++)
    {
        for (int x = 0; x < CHUNK_SIZE_X; x++)
            ComputeRayAtPosition(world, chunk, x, z);
    }
}

static inline int GetRay(Chunk* chunk, int x, int z)
{
    return chunk->rays[z * CHUNK_SIZE_X + x];
}

// Returns the ray at the given position relative to the chunk. This function assumes that if 
// the location is out of bounds, it will be out of bounds only by a distance of 1 block.
static inline int GetRaySafe(World* world, Chunk* chunk, int x, int z)
{
    RebasedPos p = Rebase(world, chunk->lcPos, x, z);
    return GetRay(p.chunk, p.rX, p.rZ);
}

static inline uint8_t GetSunlight(Chunk* chunk, int x, int y, int z)
{
    if (y >= GetRay(chunk, x, z))
        return MAX_LIGHT;

    return chunk->sunlight[x + CHUNK_SIZE_X * (y + CHUNK_SIZE_Y * z)];
}

static uint8_t GetSunlightSafe(World* world, Chunk* chunk, int x, int y, int z)
{
    if (y < 0) return MIN_LIGHT;

    RebasedPos p = Rebase(world, chunk->lcPos, x, z);
    return GetSunlight(p.chunk, p.rX, y, p.rZ);
}

static inline uint8_t GetLight(Chunk* chunk, int x, int y, int z)
{
    return chunk->lights[x + CHUNK_SIZE_X * (y + CHUNK_SIZE_Y * z)];
}

static uint8_t GetLightSafe(World* world, Chunk* chunk, int x, int y, int z)
{
    if (y < 0 || y >= CHUNK_SIZE_Y) return MIN_LIGHT;

    RebasedPos p = Rebase(world, chunk->lcPos, x, z);
    return GetLight(p.chunk, p.rX, y, p.rZ);
}

// Returns the highest point in the world between this column and each neighbor column.
static int ComputeMaxY(World* world, Chunk* chunk, int x, int z, int ray)
{
    ray = Max(ray, GetRaySafe(world, chunk, x - 1, z));
    ray = Max(ray, GetRaySafe(world, chunk, x + 1, z));
    ray = Max(ray, GetRaySafe(world, chunk, x, z - 1));
    ray = Max(ray, GetRaySafe(world, chunk, x, z + 1));
    
    return ray;
}

static bool SetMaxSunlight(Chunk* chunk, uint8_t light, int x, int y, int z) 
{
    if (y >= GetRay(chunk, x, z)) 
        return false;
    
    int index = x + CHUNK_SIZE_X * (y + CHUNK_SIZE_Y * z);
    uint8_t oldLight = chunk->sunlight[index];
    
    if (oldLight < light) 
    {
        chunk->sunlight[index] = light;
        return true;
    }
    
    return false;
}

static void ScatterSunlightNodes(World* world, queue<LightNode>& nodes) 
{
    world->scatterMutex.lock();

    while (nodes.size() > 0)
    {
        LightNode node = nodes.front();
        nodes.pop();

        Chunk* chunk = node.chunk;
        RelPos p = node.pos;

        Block block = GetBlockSafe(world, chunk, p.x, p.y, p.z);
        int step = GetBlockLightStep(world, block);
        int light = GetSunlight(chunk, p.x, p.y, p.z) - step;

        assert(light <= MAX_LIGHT);
        
        if (light <= MIN_LIGHT) 
            continue;
        
        for (int i = 0; i < 6; i++)
        {
            RelPos nextPos = p + DIRECTIONS_3D[i];

            RebasedPos rel = Rebase(world, chunk->lcPos, nextPos.x, nextPos.z);
            block = GetBlock(rel.chunk, rel.rX, nextPos.y, rel.rZ);

            if (IsTransparent(world, block) && SetMaxSunlight(rel.chunk, (uint8_t)light, rel.rX, nextPos.y, rel.rZ)) 
            {
                node = { rel.chunk, ivec3(rel.rX, nextPos.y, rel.rZ) };
                nodes.push(node);
            }
        }
    }

    world->scatterMutex.unlock();
}

static void ScatterSunlight(World* world, Chunk* chunk, queue<LightNode>& nodes)
{
    for (int z = 0; z < CHUNK_SIZE_X; z++)
    {
        for (int x = 0; x < CHUNK_SIZE_X; x++)
        {
            int ray = GetRay(chunk, x, z);
            int maxY = ComputeMaxY(world, chunk, x, z, ray);

            for (int y = ray; y <= maxY; y++) 
            {
                if (y >= CHUNK_SIZE_Y) continue;

                LightNode node = { chunk, ivec3(x, y, z) };
                nodes.push(node);
            }
        }
    }

    ScatterSunlightNodes(world, nodes);
}

static void GenerateLight(World* world, Chunk* chunk)
{
    ComputeRays(world, chunk);
    queue<LightNode> sunNodes;
    ScatterSunlight(world, chunk, sunNodes);
    chunk->state = CHUNK_SCATTERED;
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
                    Mesh* mesh = chunk->meshes[type];

                    if (mesh == nullptr)
                    {
                        mesh = CreateMesh();
                        chunk->meshes[type] = mesh;
                    }

                    BuildFunc(world, block)(world, chunk, mesh, x, y, z, block);
                }
            }
        }
    }

    chunk->state = CHUNK_NEEDS_FILL;
}

static inline void FillChunkMeshes(Chunk* chunk)
{
    VertexSpec spec = { true, 3, true, 3, true, 4 };

    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
        Mesh* mesh = chunk->meshes[i];

        if (mesh == nullptr) continue;
        
        if (mesh->vertCount > 0)
            FillMeshData(mesh, GL_DYNAMIC_DRAW, spec);
    }
}

static inline void BuildChunkNow(World* world, Chunk* chunk)
{
    BuildChunk(world, chunk);
    FillChunkMeshes(chunk);
    chunk->state = CHUNK_BUILT;
}

static inline void DestroyChunkMeshes(Chunk* chunk)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
       DestroyMesh(chunk->meshes[i]);
       chunk->meshes[i] = nullptr;
    }
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
        cam->meshLists[i].clear();

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
                    chunk->state = CHUNK_SCATTERING;
                    QueueAsync(state, GenerateLight, world, chunk);
                }
            } break;

            case CHUNK_SCATTERED:
            {
                if (NeighborsHaveState(world, chunk, CHUNK_SCATTERED))
                {
                    chunk->state = CHUNK_BUILDING;
                    world->chunksBuilding++;
                    QueueAsync(state, BuildChunk, world, chunk);
                }
            } break;

            case CHUNK_NEEDS_FILL:
            {
                assert(chunk->state == CHUNK_NEEDS_FILL);
                FillChunkMeshes(chunk);
                chunk->state = CHUNK_BUILT;
                world->chunksBuilding--;
            }

            case CHUNK_BUILT:
            {
                if (chunk->pendingUpdate)
                {
                    DestroyChunkMeshes(chunk);
                    BuildChunkNow(world, chunk);
                    chunk->pendingUpdate = false;
                }

                for (int m = 0; m < CHUNK_MESH_COUNT; m++)
                {
                    Mesh* mesh = chunk->meshes[m];

                    if (mesh != nullptr && mesh->vertCount > 0)
                    {
                        ChunkMesh cM = { mesh, (vec3)chunk->lwPos };
                        cam->meshLists[m].push_back(cM);
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

// Look up table for light values. This is computed by normalizing the value range [1, 15] to [0, 1], taking the square
// root, and then taking the value to the 2.2 power. 
static const float lightOutput[] = { 0.06f, 0.12f, 0.25f, 0.36f, 0.45f, 0.51f, 0.58f, 0.63f, 0.68f, 0.73f, 0.77f, 0.81f, 0.85f, 0.89f, 0.93f, 0.97f };

static inline Color GetFinalLight(World* world, Chunk* chunk, RelPos pos)
{
    float light = lightOutput[GetLightSafe(world, chunk, pos.x, pos.y, pos.z)];
    float sun = lightOutput[GetSunlightSafe(world, chunk, pos.x, pos.y, pos.z)];

    return Color(light, light, light, sun);
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
        Color c1 = GetFinalLight(world, chunk, a);
        Color c2 = GetFinalLight(world, chunk, b);
        Color c3 = GetFinalLight(world, chunk, c);
        Color c4 = GetFinalLight(world, chunk, d);

        return Average(c1, c2, c3, c4);
    }
    else 
    {
        Color c1 = GetFinalLight(world, chunk, a);
        Color c2 = GetFinalLight(world, chunk, b);
        Color c3 = GetFinalLight(world, chunk, c);

        return Average(c1, c2, c3);
    }
}

#define VERTEX_LIGHT(a, o1, o2, o3) VertexLight(world, chunk, AXIS_##a, rP, o1, o2, o3)

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
        float tex = textures[FACE_TOP];
        SetMeshIndices(mesh, 10);
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Y, 1, 1, -1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Y, 1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Y, -1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Y, -1, 1, -1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi - 1, zi)))
    {
        float tex = textures[FACE_BOTTOM];
        SetMeshIndices(mesh, 10);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Y, -1, -1, -1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Y, -1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Y, 1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Y, 1, -1, -1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi, zi + 1)))
    {
        float tex = textures[FACE_FRONT];
        SetMeshIndices(mesh, 10);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Z, -1, -1, 1)); 
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Z, -1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Z, 1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Z, 1, -1, 1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi, yi, zi - 1)))
    {
        float tex = textures[FACE_BACK];
        SetMeshIndices(mesh, 10);
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Z, 1, -1, -1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Z, 1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Z, -1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Z, -1, -1, -1)); 
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi + 1, yi, zi)))
    {
        float tex = textures[FACE_RIGHT];
        SetMeshIndices(mesh, 10);
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(X, 1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(X, 1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(X, 1, 1, -1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(X, 1, -1, -1));
    }

    if (CanDrawFace(world, cull, GetBlockSafe(world, chunk, xi - 1, yi, zi)))
    {
        float tex = textures[FACE_LEFT];
        SetMeshIndices(mesh, 10);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(X, -1, -1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(X, -1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(X, -1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(X, -1, -1, 1));
    }
}
