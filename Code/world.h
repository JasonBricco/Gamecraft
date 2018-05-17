// Voxel Engine
// Jason Bricco

// Chunk size in blocks.
#define CHUNK_SIZE 32

// Size with padding. Padding removes dependency on neighboring chunks.
#define PADDED_CHUNK_SIZE 34
#define CHUNK_SIZE_BITS 5

// Full size with padding.
#define CHUNK_SIZE_3 39304

#define CHUNK_HASH_SIZE 4096
#define SEA_LEVEL 12

typedef uint16_t Block;

typedef ivec3 LChunkPos;
typedef ivec3 LWorldPos;
typedef ivec3 ChunkPos;
typedef ivec3 WorldPos;
typedef ivec3 RelPos;

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
    CHUNK_GENERATING = 0,
    CHUNK_GENERATED = 1,
    CHUNK_BUILDING = 2,
    CHUNK_NEEDS_FILL = 3,
    CHUNK_BUILT = 4,
    CHUNK_UPDATE = 5
};

struct Chunk
{
    LChunkPos lcPos;
    ChunkPos cPos;
    LWorldPos lwPos;

    Block blocks[CHUNK_SIZE_3];

    Mesh* meshes[CHUNK_MESH_COUNT];

    ChunkState state;

    bool active;

    Chunk* next;
};

struct ChunkQueue
{
    Chunk* front;
    Chunk* end;
    int count;
};

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
