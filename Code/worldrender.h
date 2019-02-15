//
// Jason Bricco
//

struct GameState;

static Chunk** GetVisibleChunks(World* world, Camera* cam, int& visibleCount);
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block);
static void ProcessVisibleChunks(GameState* state, World* world, Camera* cam, Chunk** visibleChunks, int visibleCount);
