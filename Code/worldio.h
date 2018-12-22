//
// Jason Bricco
//

// Represents block data to be written to disk.
union SerializedBlock
{
    int32_t value;

    struct
    {
        Block block;
        uint8_t light, sunlight;
    };
};

struct SerializedChunk
{
    int size, maxSize;
    uint16_t* data;

    inline void Add(uint16_t value)
    {
        if (size + 1 > maxSize)
        {
            maxSize = (maxSize + 1) * 2;
            data = Realloc<uint16_t>(data, maxSize);
            assert(data != nullptr);
        }

        data[size++] = value;
    }

    inline void Reserve(int count)
    {
        maxSize = count;
        data = Realloc<uint16_t>(data, count);
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
