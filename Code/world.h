//
// Jason Bricco
//

// Chunk size in blocks.
#define CHUNK_SIZE 32

// Size with padding. Padding removes dependency on neighboring chunks.
#define PADDED_CHUNK_SIZE 34
#define CHUNK_SIZE_BITS 5

// Full size with padding.
#define CHUNK_SIZE_3 39304

#define CHUNK_HASH_SIZE 4096
#define SEA_LEVEL 12

// Number of chunks on each dimensions in a region file.
#define REGION_SIZE 8
#define REGION_SIZE_3 512
#define REGION_SHIFT 3
#define REGION_MASK 7

typedef uint16_t Block;

// The position of the chunk in local space around the player.
// All loaded chunks are in a local array. This indexes into it.
typedef ivec3 LChunkPos;

// The world (block) position within the local space around the player.
typedef ivec3 LWorldPos;

// The actual position of the chunk in the entire world space.
// This allows the chunk to store its intended position irrespective of where it is
// in the array of loaded chunks.
typedef ivec3 ChunkPos;

// The block position in actual world space.
typedef ivec3 WorldPos;

// A position relative to a specific chunk.
typedef ivec3 RelPos;

// The position in terms of chunk region files.
typedef ivec3 RegionPos;

typedef FastNoiseSIMD Noise;

enum BlockType : Block
{
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_WATER,
    BLOCK_CRATE,
    BLOCK_SAND,
    BLOCK_STONEBRICK,
    BLOCK_COUNT
};

enum BlockFace
{
    FACE_TOP,
    FACE_BOTTOM,
    FACE_FRONT,
    FACE_BACK,
    FACE_RIGHT,
    FACE_LEFT
};

enum CullType
{
    CULL_OPAQUE,
    CULL_TRANSPARENT,
    CULL_ALL
};

enum ChunkState
{
    CHUNK_LOADING,
    CHUNK_LOADED,
    CHUNK_BUILDING,
    CHUNK_NEEDS_FILL,
    CHUNK_BUILT,
    CHUNK_UPDATE,
    CHUNK_STATE_COUNT
};

struct Chunk
{
    LChunkPos lcPos;
    ChunkPos cPos;
    LWorldPos lwPos;

    Block blocks[CHUNK_SIZE_3];

    Mesh* meshes[CHUNK_MESH_COUNT];

    ChunkState state;

    bool active, modified;

    Chunk* next;
};

struct ChunkQueue
{
    Chunk* front;
    Chunk* end;
    int count;
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

typedef SerializedChunk* Region;
typedef unordered_map<RegionPos, Region, ivec3Key, ivec3Key> RegionMap;

struct World
{
    // Size of the active world near the origin. 
    // The potential world is unlimited in size.
    int sizeH, sizeV;

    // Chunk pool to avoid constant allocating/freeing.
    Chunk** pool;
    int poolSize, maxPoolSize;

    // All actively loaded chunks around the player.
    Chunk** chunks;
    int totalChunks;

    // Chunks to be rendered.
    Chunk** visibleChunks;
    int visibleCount;

    // Chunk hash table to store chunks that need to transition.
    Chunk* chunkHash[CHUNK_HASH_SIZE];

    // Chunks currently awaiting destruction.
    ChunkQueue destroyQueue;

    // Spawn and reference corner in world chunk coordinates.
    ChunkPos spawnChunk;
    ChunkPos ref;

    // Specifies the area in float space that the player exists within.
    // This is the area within the center local chunk.
    Rectf pBounds;

    // Path to the world saves folder.
    char* savePath;

    // Maps a region position with the grid of chunks that exist within it.
    RegionMap regions;

    // Used to ensure blocking when loading a region file.
    mutex regionMutex;

    // The region the player is in.
    RegionPos playerRegion;

    int seed;
};

using BuildBlockFunc = void(*)(Chunk*, Mesh*, int, int, int, Block);

struct BlockData
{
    float textures[6];
    BlockMeshType meshType;
    CullType cull;
    BuildBlockFunc buildFunc;
};
