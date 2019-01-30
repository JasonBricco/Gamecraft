//
// Jason Bricco
//

struct SerializedChunk
{
    int size, maxSize;
    uint16_t* data;

    inline void Add(uint16_t value)
    {
        if (size + 1 > maxSize)
        {
            maxSize = (maxSize + 1) * 2;
            data = (uint16_t*)realloc(data, maxSize * sizeof(uint16_t));
            assert(data != nullptr);
        }

        data[size++] = value;
    }

    inline void Reserve(int count)
    {
        maxSize = count;
        data = (uint16_t*)realloc(data, count * sizeof(uint16_t));
        assert(data != nullptr);
    }

    inline void Clear()
    {
        size = 0;
    }
};

struct Region
{
    SerializedChunk* chunks;
    bool hasData;
};

static bool LoadChunkFromDisk(World* world, Chunk* chunk);
static void SaveChunk(World* world, Chunk* chunk);
static void DeleteRegions(World* world);
