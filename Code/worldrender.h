//
// Gamecraft
//

struct GameState;

static void GetVisibleChunks(World* world, Camera* cam);
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block);
static void ProcessVisibleChunks(GameState* state, World* world, Renderer& rend);
static void DestroyChunkMeshes(Chunk* chunk);
