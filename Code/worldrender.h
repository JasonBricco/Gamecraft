//
// Jason Bricco
//

static inline void DestroyChunkMeshes(Chunk* chunk);
static void GetVisibleChunks(World* world, Camera* cam);
static void BuildBlock(Chunk* chunk, Mesh* mesh, int xi, int yi, int zi, Block block);
static void TryBuildMeshes(World* world, Renderer* rend);
