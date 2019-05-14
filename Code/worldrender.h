//
// Gamecraft
//

struct GameState;

static void WorldRenderUpdate(GameState* state, World* world, Camera* cam);
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block);
static void PrepareWorldRender(GameState* state, World* world, Renderer& rend);
static void DestroyChunkMeshes(Chunk* chunk);
static void ReturnChunkMeshes(Renderer& rend, Chunk* chunk);
