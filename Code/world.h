// Voxel Engine
// Jason Bricco

// Chunk size in blocks.
#define CHUNK_SIZE 16
#define CHUNK_SIZE_BITS 4
#define WORLD_HEIGHT 128
#define CHUNK_SIZE_3 32768
#define CHUNK_HASH_SIZE 1024

typedef FastNoiseSIMD Noise;

enum ChunkState
{
	CHUNK_GENERATED = 0,
	CHUNK_BUILT = 1
};

struct Chunk
{
	// Chunk coordinates in local space (around the origin).
	int32_t cX, cZ;

	// Chunk coordinates in world space.
	int32_t wX, wZ;

	int blocks[CHUNK_SIZE_3];

	Mesh* mesh;

	ChunkState state;

	// When hash table collisions occur, chunks will be chained together in 
	// their shared bucket using this pointer (external chaining).
	Chunk* nextInHash;
};

struct World
{
	Noise* noise;

	// Width of the active world near the origin. 
	// The potential world is unlimited in size.
	int width;

	// Chunk pool to avoid constant allocating/freeing.
	Chunk** pool;
	int poolSize;

	// Actively loaded chunks around the player.
	Chunk** chunks;
	int totalChunks;

	// Chunk hash table to store chunks that need to transition.
	Chunk* chunkHash[CHUNK_HASH_SIZE];

	// Spawn and reference corner in world chunk coordinates.
	int spawnX, spawnZ;
	int refX, refZ;

	// Specifies the area in float space that the player exists within.
	// This is the area within the center local chunk.
	float pMin, pMax;
};

__forceinline Vec3i ToLocalPos(Vec3i wPos);
inline Vec3i ToChunkPos(Vec3i wPos);
inline Vec3i ToChunkPos(Vec3 wPos);

inline int ChunkIndex(World* world, int cX, int cZ);

inline Chunk* GetChunk(World* world, int cX, int cZ);
inline Chunk* GetChunk(World* world, Vec3i cPos);

inline void MoveChunk(World* world, Chunk* chunk, int toX, int toZ);

inline uint32_t ChunkHashBucket(int32_t x, int32_t z);

// Checks to see if the chunk from the hash table is the one matching
// the coordinates specified.
inline bool IsCorrect(Chunk* chunk, int32_t wX, int32_t wZ);

// The chunk pool stores existing chunks during a world shift. Chunks will be
// pulled out from the pool to fill the new world section if applicable.
// Otherwise, new chunks will be created and the ones remaining in the pool
// will be destroyed.
static Chunk* ChunkFromHash(World* world, uint32_t bucket, int32_t wX, int32_t wZ);
inline Chunk* ChunkFromHash(World* world, int32_t wX, int32_t wZ);
inline void AddChunkToHash(World* world, Chunk* chunk);

inline void SetBlock(Chunk* chunk, int lX, int lY, int lZ, int block);
inline void SetBlock(Chunk* chunk, Vec3i lPos, int block);

inline void SetBlock(World* world, int wX, int wY, int wZ, int block, bool update);
inline void SetBlock(World* world, Vec3i wPos, int block, bool update);

inline int GetBlock(Chunk* chunk, Vec3i lPos);
inline int GetBlock(Chunk* chunk, int lX, int lY, int lZ);

inline int GetBlock(World* world, Vec3i pos);
static int GetBlock(World* world, int wX, int wY, int wZ);

static void GenerateChunkTerrain(Noise* noise, Chunk* chunk, int startX, int startZ);

// Fill a chunk with a single block type.
static void FillChunk(Chunk* chunk, int block);

inline void AddChunkToPool(World* world, Chunk* chunk);
inline Chunk* ChunkFromPool(World* world);

static Chunk* CreateChunk(World* world, int cX, int cZ, int wX, int wZ);
static void DestroyChunk(World* world, Chunk* chunk);

// Builds mesh data for a single block.
static void BuildBlock(World* world, Mesh* mesh, float x, float y, float z, 
	int wX, int wY, int wZ);

// Builds mesh data for the chunk.
static void BuildChunk(World* world, Chunk* chunk);

// Rebuilds chunk meshes.
inline void UpdateChunkDirect(World* world, Chunk* chunk);
inline void UpdateChunk(World* world, Chunk* chunk, Vec3i lPos);

// To allow "infinite" terrain, the world is always located near the origin.
// This function fills the world near the origin based on the reference
// world position within the world.
static void ShiftWorld(World* world);

static World* NewWorld(int loadRange);
