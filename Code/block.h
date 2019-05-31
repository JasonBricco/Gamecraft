//
// Gamecraft
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
    BLOCK_ICE,
    BLOCK_LANTERN,
    BLOCK_TRAMPOLINE,
    BLOCK_CACTUS,
    BLOCK_MAGMA,
    BLOCK_COOLED_MAGMA,
    BLOCK_OBSIDIAN,
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

enum BlockSurface
{
    SURFACE_NORMAL,
    SURFACE_ICE,
    SURFACE_WATER
};

enum BlockCollider
{
    COLLIDER_NORMAL,
    COLLIDER_BOUNCE
};

using BuildBlockFunc = void(*)(World*, Chunk*, MeshData*, int, int, int, Block);

struct BlockAnimation
{
    vector<int> frames;
    float framesPerSec, timeLeft;
    int index;
};

struct BlockData
{
    uint16_t textures[6];
    bool passable;
    BlockMeshType meshType;
    CullType cull;
    BuildBlockFunc buildFunc;
    uint8_t alpha;
    int lightEmitted, lightStep;
    Sound onSetSound;
    BlockSurface surface;
    BlockCollider collider;
    char* name;
};

static int ComputeAnimationFrame(BlockAnimation& anim, float deltaTime);
