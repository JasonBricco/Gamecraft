// Voxel Engine
// Jason Bricco

#define CHUNK_MESH_COUNT 2

enum BlockMeshType
{
    MESH_TYPE_OPAQUE,
    MESH_TYPE_FLUID
};

struct Mesh
{
    GLuint vb, ib, va;

    float* vertices;
    int* indices;

    int vertMax, indexMax;
    int vertCount, indexCount;

    vec3 lwPos;
};
