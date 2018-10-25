//
// Jason Bricco
//

static inline void DestroyChunkMeshes(Chunk* chunk)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
       DestroyMesh(chunk->meshes[i]);
       chunk->meshes[i] = nullptr;
    }
}

static void TryBuildMeshes(World* world, Renderer* rend)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
        rend->meshLists[i].clear();

    for (int i = 0; i < world->visibleCount; i++)
    {
        Chunk* chunk = world->visibleChunks[i];
        assert(chunk->active);

        switch (chunk->state)
        {
            case CHUNK_LOADED:
            {
                chunk->state = CHUNK_BUILDING;
                QueueAsync(BuildChunk, world, chunk);
                break;
            }

            case CHUNK_NEEDS_FILL:
            {
                FillMeshData(chunk->meshes);
                chunk->state = CHUNK_BUILT;
                break;
            }

            case CHUNK_UPDATE:
            {
                DestroyChunkMeshes(chunk);
                BuildChunkNow(world, chunk);
            }

            case CHUNK_BUILT:
            {
                for (int m = 0; m < CHUNK_MESH_COUNT; m++)
                {
                    Mesh* mesh = chunk->meshes[m];

                    if (mesh != nullptr && mesh->vertCount > 0)
                    {
                        mesh->lwPos = (vec3)chunk->lwPos;
                        rend->meshLists[m].push_back(mesh);
                    }
                }

                break;
            }

            default:
                break;
        }
    }
}

#define FRUSTUM_CULLING 1

static void GetVisibleChunks(World* world, Camera* cam)
{    
    for (int i = 0; i < world->totalChunks; i++)
    {
        Chunk* chunk = world->chunks[i];
        LChunkPos cP = chunk->lcPos;
        vec3 min = cP * CHUNK_SIZE;
        vec3 max = min + (CHUNK_SIZE - 1.0f);

#if FRUSTUM_CULLING 
        FrustumVisibility visibility = TestFrustum(cam, min, max);

        if (visibility >= FRUSTUM_VISIBLE)
            world->visibleChunks[world->visibleCount++] = chunk;
#else
        world->visibleChunks[world->visibleCount++] = chunk;
#endif
    }
}

// Returns true if the current block should draw its face when placed
// next to the adjacent block.
static inline bool CanDrawFace(CullType cur, Block adjBlock)
{
    CullType adj = g_blockData[adjBlock].cull;

    if (cur == CULL_OPAQUE)
        return adj >= CULL_TRANSPARENT;
    else return adj == CULL_ALL;
}

#define BLOCK_TRANSPARENT(pos) g_blockData[GetBlock(chunk, pos)].cull >= CULL_TRANSPARENT

static inline Color VertexLight(Chunk* chunk, Axis axis, RelPos pos, int dx, int dy, int dz)
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
        Color c1 = GetBlock(chunk, a) == BLOCK_AIR ? vec4(1.0f) : vec4(0.25f);
        Color c2 = GetBlock(chunk, b) == BLOCK_AIR ? vec4(1.0f) : vec4(0.25f);
        Color c3 = GetBlock(chunk, c) == BLOCK_AIR ? vec4(1.0f) : vec4(0.25f);

        return Average(c1, c2, c3);
    }
}

#define VERTEX_LIGHT(a, o1, o2, o3) cull >= CULL_TRANSPARENT ? vec4(1.0f) : VertexLight(chunk, AXIS_##a, rP, o1, o2, o3)

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
static void BuildBlock(Chunk* chunk, Mesh* mesh, int xi, int yi, int zi, Block block)
{
    float* textures = g_blockData[block].textures;

    RelPos rP = ivec3(xi, yi, zi);
    float x = (float)xi, y = (float)yi, z = (float)zi;

    CullType cull = g_blockData[block].cull;

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi + 1, zi)))
    {
        float tex = textures[FACE_TOP];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Y, 1, 1, -1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Y, 1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Y, -1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Y, -1, 1, -1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi - 1, zi)))
    {
        float tex = textures[FACE_BOTTOM];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Y, -1, -1, -1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Y, -1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Y, 1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Y, 1, -1, -1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi, zi + 1)))
    {
        float tex = textures[FACE_FRONT];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Z, -1, -1, 1)); 
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Z, -1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Z, 1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Z, 1, -1, 1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi, zi - 1)))
    {
        float tex = textures[FACE_BACK];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Z, 1, -1, -1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Z, 1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Z, -1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Z, -1, -1, -1)); 
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi + 1, yi, zi)))
    {
        float tex = textures[FACE_RIGHT];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(X, 1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(X, 1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(X, 1, 1, -1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(X, 1, -1, -1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi - 1, yi, zi)))
    {
        float tex = textures[FACE_LEFT];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(X, -1, -1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(X, -1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(X, -1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(X, -1, -1, 1));
    }
}
