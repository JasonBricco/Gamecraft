// Voxel Engine
// Jason Bricco

// Chunk size in blocks.
#define CHUNK_SIZE 32
#define CHUNK_SIZE_BITS 5
#define CHUNK_SIZE_3 32768
#define CHUNK_HASH_SIZE 4096

#define SEA_LEVEL 12

typedef ivec3 LChunkPos;
typedef ivec3 LWorldPos;
typedef ivec3 ChunkPos;
typedef ivec3 WorldPos;
typedef ivec3 RelPos;

typedef FastNoiseSIMD Noise;

enum ChunkState
{
	CHUNK_GENERATED = 0,
	CHUNK_BUILT = 1
};

struct Chunk
{
	LChunkPos lcPos;
	ChunkPos cPos;
	LWorldPos lwPos;

	int blocks[CHUNK_SIZE_3];

	Mesh* mesh;

	ChunkState state;

	bool active;
};

struct World
{
	// Size of the active world near the origin. 
	// The potential world is unlimited in size.
	int sizeH, sizeV;

	// Chunk pool to avoid constant allocating/freeing.
	Chunk** pool;
	int poolSize, maxPoolSize;

	// Actively loaded chunks around the player.
	Chunk** chunks;
	int totalChunks;

	// Chunk hash table to store chunks that need to transition.
	Chunk* chunkHash[CHUNK_HASH_SIZE];

	// Spawn and reference corner in world chunk coordinates.
	ivec3 spawnChunk;
	ivec3 ref;

	// Specifies the area in float space that the player exists within.
	// This is the area within the center local chunk.
	Rectf pBounds;

	BlockData blockData[BLOCK_COUNT];

	Noise* noise;
};

inline RelPos ToLocalPos(int lwX, int lwY, int lwZ);
inline RelPos ToLocalPos(LWorldPos wPos);

inline LChunkPos ToChunkPos(int lwX, int lwY, int lwZ);
inline LChunkPos ToChunkPos(LWorldPos pos);
inline ivec3 ToChunkPos(vec3 wPos);

inline int ChunkIndex(World* world, int lcX, int lcY, int lcZ);

inline Chunk* GetChunk(World* world, int lcX, int lcY, int lcZ);
inline Chunk* GetChunk(World* world, LChunkPos pos);

inline uint32_t ChunkHashBucket(ivec3 pos);

// The chunk pool stores existing chunks during a world shift. Chunks will be
// pulled out from the pool to fill the new world section if applicable.
// Otherwise, new chunks will be created and the ones remaining in the pool
// will be destroyed.
static Chunk* ChunkFromHash(World* world, uint32_t bucket, ChunkPos cPos);
inline Chunk* ChunkFromHash(World* world, int32_t wX, int32_t wY, int32_t wZ);

inline void AddChunkToHash(World* world, Chunk* chunk);

inline void SetBlock(Chunk* chunk, int lX, int lY, int lZ, int block);
inline void SetBlock(Chunk* chunk, ivec3 lPos, int block);

inline void SetBlock(World* world, int wX, int wY, int wZ, int block, bool update);
inline void SetBlock(World* world, ivec3 wPos, int block, bool update);

inline int GetBlock(Chunk* chunk, RelPos pos);
inline int GetBlock(Chunk* chunk, int rX, int rY, int rZ);

inline int GetBlock(World* world, LWorldPos pos);
static int GetBlock(World* world, int lwX, int lwY, int lwZ);

inline float GetNoiseValue2D(float* noiseSet, int index);
inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z);

static void GenerateChunkTerrain(Noise* noise, Chunk* chunk, ivec3 start);

// Fill a chunk with a single block type.
static void FillChunk(Chunk* chunk, int block);

inline void AddChunkToPool(World* world, Chunk* chunk);
inline Chunk* ChunkFromPool(World* world);

static Chunk* CreateChunk(World* world, int cX, int cY, int cZ, 
	int wX, int wY, int wZ);
static void DestroyChunk(World* world, Chunk* chunk);

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
inline void BuildBlock(World* world, Mesh* mesh, float x, float y, float z, 
	int wX, int wY, int wZ, int block);

// Builds mesh data for the chunk.
static void BuildChunk(World* world, Chunk* chunk);

// Rebuilds chunk meshes.
inline void UpdateChunkDirect(World* world, Chunk* chunk);
inline void UpdateChunk(World* world, Chunk* chunk, ivec3 lPos);

// To allow "infinite" terrain, the world is always located near the origin.
// This function fills the world near the origin based on the reference
// world position within the world.
static void ShiftWorld(World* world);

static World* NewWorld(int loadRangeH, int loadRangeV);
