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

#define GROUP_DESTROY_LIMIT 6

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

enum GroupState
{
    GROUP_DEFAULT,
    GROUP_LOADED,
    GROUP_PREPROCESSING,
    GROUP_PREPROCESSED
};

struct ChunkGroup;

struct Chunk
{
    LChunkP lcPos;
    LWorldP lwPos;

    Block blocks[CHUNK_SIZE_3];
    int totalVertices;

    Mesh meshes[MESH_TYPE_COUNT];
    MeshData* meshData[MESH_TYPE_COUNT];

    bool pendingUpdate, hasMeshes, modified;
    ChunkState state;

    ChunkGroup* group;
};

struct ChunkGroup
{
    ChunkP pos;
    Chunk chunks[WORLD_CHUNK_HEIGHT];

    GroupState state;
    bool active, pendingDestroy;
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
    WorldLocation homePos;
};

struct World
{
    // Size of the active world near the origin.
    // The potential world is unlimited in size.
    int size;

    int loadRange;

    // The radius at which the terrain begins falling off into sea.
    int falloffRadius;

    ObjectPool<ChunkGroup> groupPool;
    ObjectPool<Region> regionPool;

    // All actively loaded chunk groups around the player.
    ChunkGroup** groups;
    int totalGroups;
    int workCount;
    
    vector<ivec4> groupsToCreate;
    vector<ChunkGroup*> groupsToProcess;
    
    // Chunk hash table to store chunks that need to transition.
    ChunkGroup* groupHash[GROUP_HASH_SIZE];

    vector<Chunk*> visibleChunks;
    vector<Chunk*> chunksToRebuild;

    // Chunks currently awaiting destruction.
    queue<ChunkGroup*> destroyQueue;

    // Spawn and reference corner in world chunk coordinates.
    ChunkP spawnGroup;
    ChunkP ref;

    // Specifies the area in float space that the player exists within.
    // This is the area within the center local chunk.
    Rectf pBounds;

    // Path to the world saves folder.
    char* savePath;

    // Doubly-linked list of loaded regions.
    list<Region*> regions;

    CRITICAL_SECTION regionCS;
    CONDITION_VARIABLE regionsEmpty;

    bool chunksRebuilding;

    Player* player;

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

struct NeighborBlocks
{
    RebasedPos up, down;
    RebasedPos front, back;
    RebasedPos left, right;
};

struct WorldConfig
{
    char radiusBuffer[10];
    int radius;
    bool infinite;
    BiomeType biome;

    float errorTime;
    char* error;
};
