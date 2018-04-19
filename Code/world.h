// Voxel Engine
// Jason Bricco

// Chunk size in blocks.
#define CHUNK_SIZE 16
#define CHUNK_SIZE_BITS 4
#define WORLD_BLOCK_HEIGHT 128
#define CHUNK_SIZE_3 32768

// World limits in chunks.
#define WORLD_MAX_H 512

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
