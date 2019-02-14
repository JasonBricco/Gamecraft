//
// Jason Bricco
//

// Number of chunks on each dimensions in a region file.
#define REGION_SIZE 16
#define REGION_SIZE_3 256
#define REGION_SHIFT 4
#define REGION_MASK 15
#define MAX_REGIONS 8

struct SerializedChunk
{
    int size, maxSize;
    uint16_t* data;

    inline void Add(uint16_t value)
    {
        if (size + 1 > maxSize)
        {
            maxSize = (maxSize + 1) * 2;
            data = ReallocArray(data, maxSize, uint16_t);
            assert(data != nullptr);
        }

        data[size++] = value;
    }

    inline void Reserve(int count)
    {
        maxSize = count;
        data = ReallocArray(data, count, uint16_t);
        assert(data != nullptr);
    }

    inline void Clear()
    {
        size = 0;
    }
};

struct Region
{
    RegionPos pos;
    SerializedChunk chunks[REGION_SIZE_3];
    bool hasData, modified;
    Region* next;
    Region* prev;
};

static bool LoadChunkFromDisk(World* world, Chunk* chunk);
static void SaveChunk(World* world, Chunk* chunk);
static void DeleteRegions(World* world);
