//
// Gamecraft
//

#define MIN_LIGHT 1
#define MAX_LIGHT 15

#define MAX_LIGHT_NODES 262144

static inline void ComputeSurface(ChunkGroup* group);
static void RecomputeLight(World* world, Chunk* chunk, int rX, int rY, int rZ);
