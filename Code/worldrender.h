//
// Jason Bricco
//

#define MAX_LIGHT 15

struct GameState;

static inline void DestroyChunkMeshes(Chunk* chunk);
static void GetVisibleChunks(World* world, Camera* cam);
static void BuildBlock(World* world, Chunk* chunk, Mesh* mesh, int xi, int yi, int zi, Block block);
static void ProcessVisibleChunks(GameState* state, World* world, Camera* cam);
