//
// Gamecraft
//

struct GameState;

static void WorldRenderUpdate(GameState* state, World* world, Camera* cam);
static inline bool ChunkOverflowed(World* world, Chunk* chunk, int x, int y, int z);
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block);
static void PrepareWorldRender(GameState* state, World* world, Renderer& rend);
static void ReturnChunkMesh(Renderer& rend, Chunk* chunk);
