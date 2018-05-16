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
	BLOCK_CRATE,
	BLOCK_SAND,
	BLOCK_STONEBRICK,
	BLOCK_COUNT
};

struct BlockData
{
	float textures[6];
};
