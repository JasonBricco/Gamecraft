//
// Gamecraft
//

#define MIN_LIGHT 1
#define MAX_LIGHT 15

struct GameState;

static inline void ComputeSurface(ChunkGroup* group);
static void WorldRenderUpdate(GameState* state, World* world, Camera* cam);
static inline bool ChunkOverflowed(World* world, Chunk* chunk, int x, int y, int z);
static void BuildBlock(World* world, Chunk* chunk, MeshData* data, int xi, int yi, int zi, Block block);
static void PrepareWorldRender(GameState* state, World* world, Renderer& rend);
static void DestroyChunkMeshes(Chunk* chunk);
static void ReturnChunkMeshes(Renderer& rend, Chunk* chunk);
