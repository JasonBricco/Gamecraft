// Voxel Engine
// Jason Bricco

// Chunk size in blocks.
#define CHUNK_SIZE 16
#define CHUNK_SIZE_BITS 4
#define WORLD_BLOCK_HEIGHT 128
#define CHUNK_SIZE_3 32768

// World limits in chunks.
#define WORLD_MAX_H 615

typedef FastNoiseSIMD Noise;

enum ChunkState
{
	CHUNK_GENERATED = 0,
	CHUNK_BUILT = 1
};

struct Chunk
{
	ivec3 cPos;
	ivec3 wPos;

	int blocks[CHUNK_SIZE_3];

	Mesh mesh;

	ChunkState state;
};

struct World
{
	ivec3 sizeInChunks;
	ivec3 sizeInBlocks;

	Noise* noise;

	int loadRangeH;

	Chunk** chunks;

	int loadedCount;
	Chunk** loadedChunks;

	vec3 spawn;
	ivec3 lastLoadPos;
};

static World* NewWorld(int width, int length);

inline int ChunkIndex(World* world, int x, int z);
inline ivec3 ToChunkPos(int x, int z);
inline ivec3 ToChunkPos(vec3 wPos);

inline bool ChunkInsideWorld(World* world, int x, int z);
inline bool ChunkInsideWorld(World* world, ivec3 pos);

inline Chunk* GetChunk(World* world, int x, int z);
inline Chunk* GetChunk(World* world, ivec3 pos);

inline void SetBlock(Chunk* chunk, int x, int y, int z, int block);
static void SetBlock(World* world, int x, int y, int z, int block);

static void GenerateChunkTerrain(Noise* noise, Chunk* chunk);
static void FillChunk(Chunk* chunk, int block);

inline void SetChunkLoaded(World* world, Chunk* chunk);
inline void SetChunkUnloaded(World* world, Chunk* chunk, int index);

inline Chunk* CreateChunk(World* world, ivec3 pos);
static Chunk* CreateChunk(World* world, int x, int z);

inline bool BlockInsideWorld(World* world, int x, int y, int z);

// Returns the block at the given world coordinates. 
// Returns air if the block is outside of the world.
inline int GetBlock(Chunk* chunk, int x, int y, int z);
static int GetBlock(World* world, int x, int y, int z);

static void BuildBlock(World* world, Chunk* chunk, float x, float y, float z, ivec3 wPos);
static void BuildChunk(World* world, Chunk* chunk);
static void UpdateChunk(World* world, Chunk* chunk);
static void DestroyChunk(World* world, Chunk* chunk, int i);

// Loads chunks around the chunk position 'pos'.
static void LoadSurroundingChunks(World* world, ivec3 pos);
static void UnloadChunks(World* world, ivec3 pos);

inline void DrawChunk(Chunk* chunk);
