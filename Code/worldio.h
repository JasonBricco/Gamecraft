//
// Gamecraft
//

// Number of chunks on each dimensions in a region file.
#define REGION_SIZE 8
#define REGION_SIZE_3 256
#define REGION_MASK 7
#define MAX_REGIONS 4

#define MAX_SERIALIZED_DATA (CHUNK_SIZE_3 * 2)

struct SerializedChunk
{
    int size;
    uint16_t data[MAX_SERIALIZED_DATA];

    inline void Add(uint16_t value)
    {
        assert(size + 1 < MAX_SERIALIZED_DATA);
        data[size++] = value;
    }

    inline void Clear()
    {
        size = 0;
    }
};

struct Region
{
    RegionP pos;
    SerializedChunk chunks[REGION_SIZE_3];
    bool hasData, modified;
    Region* next;
    Region* prev;
};

static bool LoadGroupFromDisk(World* world, ChunkGroup* group);
static void SaveGroup(GameState*, World* world, void* groupPtr);
static void DeleteRegions(World* world);
static bool LoadWorldFileData(GameState* state, World* world);
