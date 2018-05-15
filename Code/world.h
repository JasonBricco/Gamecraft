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

typedef ivec3 LChunkPos;
typedef ivec3 LWorldPos;
typedef ivec3 ChunkPos;
typedef ivec3 WorldPos;
typedef ivec3 RelPos;

typedef FastNoiseSIMD Noise;

enum ChunkState
{
	CHUNK_GENERATING = 0,
	CHUNK_GENERATED = 1,
	CHUNK_BUILDING = 2,
	CHUNK_NEEDS_FILL = 3,
	CHUNK_BUILT = 4,
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

	BlockData blockData[BLOCK_COUNT];

	int seed;
};
