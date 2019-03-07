//
// Jason Bricco
//

typedef uint16_t Block;

struct World;
struct Chunk;

enum BlockType : Block
{
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_CRATE,
    BLOCK_SAND,
    BLOCK_STONE_BRICK,
    BLOCK_METAL_CRATE,
    BLOCK_WATER,
    BLOCK_WOOD,
    BLOCK_LEAVES,
    BLOCK_CLAY,
    BLOCK_SNOW,
    BLOCK_COUNT
};

enum BlockFace
{
    FACE_TOP,
    FACE_BOTTOM,
    FACE_FRONT,
    FACE_BACK,
    FACE_RIGHT,
    FACE_LEFT
};

enum CullType
{
    CULL_OPAQUE,
    CULL_TRANSPARENT,
    CULL_ALL
};

using BuildBlockFunc = void(*)(World*, Chunk*, MeshData*, int, int, int, Block);

struct BlockData
{
    uint16_t textures[6];
    bool passable;
    BlockMeshType meshType;
    CullType cull;
    BuildBlockFunc buildFunc;
    Sound onSetSound;
    char* name;
};
