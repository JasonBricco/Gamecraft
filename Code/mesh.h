//
// Jason Bricco
//

#define CHUNK_MESH_COUNT 2

enum BlockMeshType
{
    MESH_TYPE_OPAQUE,
    MESH_TYPE_FLUID
};

struct Mesh
{
    GLuint vb, ib, va;
    int vertCount, indexCount;
};

struct MeshData
{
    float* vertices;
    int* indices;

    int vertCount, indexCount;
    int vertMax, indexMax;
};

struct VertexSpec
{
    bool position;
    int numPositions;

    bool texture;
    int numUvs;

    bool color;
    int numColors;
};
