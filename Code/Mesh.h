//
// Gamecraft
//

#define MAX_VERTICES 40000
#define MAX_INDICES 60000

enum BlockMeshType
{
    MESH_OPAQUE,
    MESH_TRANSPARENT,
    MESH_FLUID,
    MESH_MAGMA,
    MESH_TYPE_COUNT
};

enum OcclusionState
{
    OCCLUSION_HIDDEN,
    OCCLUSION_VISIBLE,
    OCCLUSION_WAITING,
    OCCLUSION_NONE
};

struct OcclusionMesh
{
    GLuint va, vertices;
    GLuint indices;
};

struct MeshIndices
{
    GLuint handle;
    int count;
};

struct Mesh
{
    GLuint va, vertices;
    MeshIndices indices[MESH_TYPE_COUNT];
    GLuint occlusionQuery;
    OcclusionState occlusionState;
    bool hasData;
};

struct MeshIndexData
{
    uint16_t data[MAX_INDICES];
    int count;
};

struct VertexInfo
{
    u8vec3 pos;
    u8vec3 uv;
    Colori color;
    uint8_t alpha;
    uint8_t reserved;
};

struct MeshData
{
    VertexInfo vertices[MAX_VERTICES];
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
