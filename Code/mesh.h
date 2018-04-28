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

static Mesh* CreateMesh();

inline void SetMeshVertex(Mesh* mesh, float x, float y, float z, 
	float u, float v, float tex, float r, float g, float b, float a);
inline void SetMeshIndices(Mesh* mesh);
static void FillMeshData(Mesh* mesh);

static void DrawMesh(Mesh* mesh);
