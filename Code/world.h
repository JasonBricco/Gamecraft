//
// Gamecraft
//

// Chunk size in blocks.
#define CHUNK_SIZE_H 32
#define CHUNK_SIZE_V 64

#define CHUNK_H_BITS 5
#define CHUNK_V_BITS 6

#define CHUNK_H_MASK 31
#define CHUNK_V_MASK 63

// The number of blocks in a chunk.
#define CHUNK_SIZE_3 65536
#define CHUNK_SIZE_2 1024

#define GROUP_HASH_SIZE 2048
#define SEA_LEVEL 12

#define WORLD_CHUNK_HEIGHT 4
#define WORLD_BLOCK_HEIGHT 256

// The position of the chunk in local space around the player.
// All loaded chunks are in a local array. This indexes into it.
typedef ivec3 LChunkP;

// The world (block) position within the local space around the player.
typedef ivec3 LWorldP;

// The actual position of the chunk in the entire world space.
// This allows the chunk to store its intended position irrespective of where it is
// in the array of loaded chunks.
typedef ivec3 ChunkP;

// The block position in actual world space.
typedef ivec3 WorldP;

// A position relative to a specific chunk.
typedef ivec3 RelP;

// The position in terms of chunk region files.
typedef ivec3 RegionP;

typedef FastNoiseSIMD Noise;

enum ChunkState
{
    CHUNK_DEFAULT,
    CHUNK_LOADED_DATA,
    CHUNK_BUILDING,
    CHUNK_NEEDS_FILL,
    CHUNK_BUILT,
    CHUNK_STATE_COUNT
};

struct Chunk
{
    LChunkP lcPos;
    LWorldP lwPos;

    Block blocks[CHUNK_SIZE_3];

    Mesh meshes[MESH_TYPE_COUNT];
    MeshData* meshData[MESH_TYPE_COUNT];

    bool pendingUpdate;

    ChunkState state;

    bool modified;
};

struct ChunkGroup
{
    ChunkP pos;
    Chunk chunks[WORLD_CHUNK_HEIGHT];
    bool active, loaded;
};

struct WorldLocation
{
    WorldP wP;
    LWorldP lP;
};

struct Player;
struct Region;

struct WorldProperties
{
    int seed, radius, biome;
};

struct World
{
    // Size of the active world near the origin.
    // The potential world is unlimited in size.
    int size;

    int loadRange;

    // The radius at which the terrain begins falling off into sea.
    int falloffRadius;

    // Chunk pool to avoid constant allocating/freeing.
    ChunkGroup** pool;
    int poolSize, maxPoolSize;

    // All actively loaded chunk groups around the player.
    ChunkGroup** groups;
    int totalGroups;

    // Tracks work being done by background threads so that the world cannot shift
    // while background work is being done.
    int buildCount;

    // Chunk hash table to store chunks that need to transition.
    ChunkGroup* groupHash[GROUP_HASH_SIZE];

    Chunk** visibleChunks;
    int visibleCount;

    // Chunks currently awaiting destruction.
    vector<ChunkGroup*> destroyList;

    // Spawn and reference corner in world chunk coordinates.
    ChunkP spawnGroup;
    ChunkP ref;

    // Specifies the area in float space that the player exists within.
    // This is the area within the center local chunk.
    Rectf pBounds;

    // Path to the world saves folder.
    char* savePath;

    // Doubly-linked list of loaded regions.
    Region* firstRegion;
    int regionCount;

    // Used to ensure blocking when loading a region file.
    CRITICAL_SECTION regionCS;

    Player* player;

    // The region the player is in.
    RegionP playerRegion;

    WorldProperties properties;

    BlockData blockData[BLOCK_COUNT];

    BlockType blockToSet;

    Biome biomes[BIOME_COUNT];

    // Cursor block information for the debug HUD.
    bool cursorOnBlock;
    ivec3 cursorBlockPos;
};

struct RebasedPos
{
    Chunk* chunk;
    int rX, rY, rZ;
};

struct WorldConfig
{
    char radiusBuffer[10];
    int radius;
    bool infinite;
    BiomeType biome;
};
