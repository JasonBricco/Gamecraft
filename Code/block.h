// Voxel Engine
// Jason Bricco

enum BlockFace
{
	FACE_TOP,
	FACE_BOTTOM,
	FACE_FRONT,
	FACE_BACK,
	FACE_RIGHT,
	FACE_LEFT
};

enum BlockType
{
	BLOCK_AIR,
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_STONE,
	BLOCK_WATER,
	BLOCK_SAND,
	BLOCK_COUNT
};

struct BlockData
{
	float textures[6];
};

inline void SetBlockTextures(BlockData& data, float top, float bottom,
	float front, float back, float right, float left);
static void CreateBlockData(BlockData* data);
