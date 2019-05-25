//
// Gamecraft
//

#define MAX_VERTICES 40000
#define MAX_INDICES 60000

enum BlockMeshType
{
    MESH_OPAQUE,
    MESH_OPAQUE_ANIMATED,
    MESH_TRANSPARENT,
    MESH_TRANSPARENT_ANIMATED,
    MESH_TYPE_COUNT
};

struct MeshIndices
{
    GLuint handle;
    int count;
};

struct Mesh
{
    GLuint va;
    GLuint positions, uvs, colors, alpha;
    MeshIndices indices[MESH_TYPE_COUNT];
    bool hasData;
};

struct MeshIndexData
{
    uint16_t data[MAX_INDICES];
    int count;
};

struct MeshData
{
    u8vec3 positions[MAX_VERTICES];
    u16vec3 uvs[MAX_VERTICES];
    Colori colors[MAX_VERTICES];
    uint8_t alpha[MAX_VERTICES];
    MeshIndexData indices[MESH_TYPE_COUNT];

    int vertCount;
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
