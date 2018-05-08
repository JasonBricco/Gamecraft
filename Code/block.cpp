// Voxel Engine
// Jason Bricco

inline void SetBlockTextures(BlockData& data, float top, float bottom, float front, float back, 
	float right, float left)
{
	data.textures[FACE_TOP] = top;
	data.textures[FACE_BOTTOM] = bottom;
	data.textures[FACE_FRONT] = front;
	data.textures[FACE_BACK] = back;
	data.textures[FACE_RIGHT] = right;
	data.textures[FACE_LEFT] = left;
}

static void CreateBlockData(BlockData* data)
{
	BlockData& grass = data[BLOCK_GRASS];
	SetBlockTextures(grass, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	
	BlockData& dirt = data[BLOCK_DIRT];
	SetBlockTextures(dirt, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f);

	BlockData& stone = data[BLOCK_STONE];
	SetBlockTextures(stone, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f);
}
