//
// Jason Bricco
//

#define CHUNK_MESH_COUNT 2

enum BlockMeshType
{
    MESH_TYPE_OPAQUE,
    MESH_TYPE_FLUID
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
    GLuint positions, texCoords, colors;
    GLuint indices;

    int32_t flags;
    int indexCount;
};

struct MeshData
{
    vec3* positions;
    vec3* texCoords;
    Color* colors;
    int* indices;

    int vertexCount, vertexMax;
    int indexCount, indexMax;

    bool valid;
};

struct MeshDataPool
{
    int capacity, count;
    MeshData** data;
};
