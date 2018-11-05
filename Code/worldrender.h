//
// Jason Bricco
//

struct GameState;

static inline void DestroyChunkMeshes(Chunk* chunk);
static void GetVisibleChunks(World* world, Camera* cam);
static void BuildBlock(World* world, Chunk* chunk, Mesh* mesh, int xi, int yi, int zi, Block block);
static void TryBuildMeshes(GameState* state, World* world, Camera* cam);
