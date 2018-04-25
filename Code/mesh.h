// Voxel Engine
// Jason Bricco

struct Mesh
{
	GLuint vb, ib, va;

	vector<float> vertices;
	vector<int> indices;
};

static void InitializeMesh(Mesh* mesh);
static void DestroyMesh(Mesh* mesh);

inline void SetMeshVertex(Mesh* mesh, float x, float y, float z, 
	float u, float v, float tex, float r, float g, float b, float a);
inline void SetMeshIndices(Mesh* mesh);
static void FillMeshData(Mesh* mesh);

static void DrawMesh(Mesh* mesh);
