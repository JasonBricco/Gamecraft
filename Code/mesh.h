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
    GLuint positions, uvs, colors;
    GLuint indices;

    int32_t flags;
    
    void* data;

    // Pointers into the data array.
    vec3* positionData;
    vec3* uvData;
    Colori* colorData;
    int* indexData;

    int vertCount, vertMax;
    int indexCount, indexMax;
};
