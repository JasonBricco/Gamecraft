// Voxel Engine
// Jason Bricco

struct Mesh
{
	GLuint vb, ib, va;

	float* vertices;
	int* indices;

	int vertMax, indexMax;
	int vertCount, indexCount;
};
