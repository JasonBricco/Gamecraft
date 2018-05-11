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

struct TerrainGenData
{
	int seed;
	Chunk* chunk;

	TerrainGenData(int s, Chunk* c) : seed(s), chunk(c) {}
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

	int seed;
};
