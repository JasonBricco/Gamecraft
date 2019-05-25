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

struct Mesh
{
    GLuint va;
    GLuint positions, uvs, colors;
    GLuint indices;
    int indexCount;
};

struct MeshData
{
    u8vec3 positions[MAX_VERTICES];
    u16vec3 uvs[MAX_VERTICES];
    Colori colors[MAX_VERTICES];
    uint16_t indices[MAX_INDICES];

    int vertCount, indexCount;
};

enum MeshFlags
{
    MESH_NO_FLAGS = 0,
    MESH_NO_UVS = 1,
    MESH_NO_COLORS = 2
};

struct Mesh2D
{
    GLuint va;
    GLuint positions, uvs, colors;
    GLuint indices;
    int32_t flags;
};

struct MeshData2D
{
    vec3 positions[4];
    u16vec2 uvs[4];
    Colori colors[4];
    uint16_t indices[6];
};
