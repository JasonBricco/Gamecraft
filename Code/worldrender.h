//
// Gamecraft
//

struct GameState;

static void GetVisibleChunks(World* world, Camera* cam);
static bool BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block);
static void ProcessVisibleChunks(GameState* state, World* world, Renderer& rend);
static void DestroyChunkMeshes(Chunk* chunk);
static void ReturnChunkMeshes(Renderer& rend, Chunk* chunk);
