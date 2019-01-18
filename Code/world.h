//
// Jason Bricco
//

// Chunk size in blocks.
#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 256

#define CHUNK_SIZE_BITS 4
#define CHUNK_MASK 15

// The number of blocks in a chunk.
#define CHUNK_SIZE_3 65536
#define CHUNK_SIZE_2 256

#define CHUNK_HASH_SIZE 4096
#define SEA_LEVEL 12

// Number of chunks on each dimensions in a region file.
#define REGION_SIZE 16
#define REGION_SIZE_3 256
#define REGION_SHIFT 4
#define REGION_MASK 15

#define WORLD_HEIGHT 256

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

enum ChunkState
{
    CHUNK_LOADING,
    CHUNK_LOADED,
    CHUNK_BUILDING,
    CHUNK_NEEDS_FILL,
    CHUNK_BUILT,
    CHUNK_STATE_COUNT
};

struct Chunk
{
    LChunkPos lcPos;
    ChunkPos cPos;
    LWorldPos lwPos;

    Block blocks[CHUNK_SIZE_3];

    Mesh meshes[CHUNK_MESH_COUNT];
    MeshData* meshData[CHUNK_MESH_COUNT];

    bool pendingUpdate;

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

struct Player;
struct Region;

typedef unordered_map<RegionPos, Region, ivec3Key, ivec3Key> RegionMap;

struct World
{
    // Size of the active world near the origin.
    // The potential world is unlimited in size.
    int size;

    int loadRange;

    // The radius of the island the terrain generates within and the radius
    // at which the terrain begins falling off into sea.
    int radius;
    int falloffRadius;

    // Chunk pool to avoid constant allocating/freeing.
    Chunk** pool;
    int poolSize, maxPoolSize;

    // All actively loaded chunks around the player.
    Chunk** chunks;
    int totalChunks;

    // Chunks to be rendered.
    Chunk** visibleChunks;
    int visibleCount;

    // Tracks work being done by background threads so that the world cannot shift
    // while background work is being done.
    int buildCount;

    vector<ivec4> chunksToCreate;

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
    HANDLE regionMutex;

    Player* player;

    // The region the player is in.
    RegionPos playerRegion;

    int seed;

    BlockData blockData[BLOCK_COUNT];

    BlockType blockToSet;

    #define MESH_POOL_CAPACITY 256
    MeshData meshData[MESH_POOL_CAPACITY];
    volatile uint32_t meshDataCount;
};

struct RebasedPos
{
    Chunk* chunk;
    int rX, rZ;
};

struct WorldConfig
{
    char radiusBuffer[10];
    int radius;
    bool infinite;
};
