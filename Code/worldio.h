//
// Gamecraft
//

// Number of chunks on each dimensions in a region file.
#define REGION_SIZE 8
#define REGION_SIZE_3 256
#define REGION_MASK 7

struct Region
{
    RegionP pos;
    List<uint16_t> chunks[REGION_SIZE_3];
    bool hasData, modified;
    Region* next;
    Region* prev;
    int activeCount;
};

static bool LoadGroupFromDisk(World* world, ChunkGroup* group);
static void SaveGroup(GameState*, World* world, void* groupPtr);
static void DeleteRegions(World* world);
static bool LoadWorldFileData(GameState* state, World* world);
static void RemoveFromRegion(World* world, ChunkGroup* group);
