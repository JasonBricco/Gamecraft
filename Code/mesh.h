//
// Gamecraft
//

#define MAX_VERTICES 40000
#define MAX_INDICES 60000

enum BlockMeshType
{
    MESH_TYPE_OPAQUE,
    MESH_TYPE_TRANSPARENT,
    MESH_TYPE_FLUID,
    MESH_TYPE_COUNT
};

enum MeshFlags
{
    MESH_NO_FLAGS = 0,
    MESH_NO_UVS = 1,
    MESH_NO_COLORS = 2
};

struct Mesh
{
    GLuint va;
    GLuint positions, uvs, colors;
    GLuint indices;

    int32_t flags;
    int indexCount;
};

struct MeshData
{
    vec3 positions[MAX_VERTICES];
    u16vec3 uvs[MAX_VERTICES];
    Colori colors[MAX_VERTICES];
    int indices[MAX_INDICES];

    int vertCount, indexCount;
};
