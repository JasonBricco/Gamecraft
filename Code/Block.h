//
// Gamecraft
//

typedef uint16_t Block;

struct World;
struct Chunk;
struct Player;

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
    BLOCK_KILL_ZONE,
    BLOCK_LAVA,
    BLOCK_GLASS,
    BLOCK_SHOCKER,
    BLOCK_COUNT
};

enum CullValue
{
    CULL_OPAQUE,
    CULL_CUTOUT,
    CULL_TRANSPARENT,
    CULL_FLUID,
    CULL_INVISIBLE
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

enum BlockSurface
{
    SURFACE_NORMAL,
    SURFACE_ICE,
    SURFACE_WATER
};

struct OverTimeDamage
{
    BlockType type;
    float interval;
    int damage;
    bool active;
    float timeLeft;
};

using BuildBlockFunc = void(*)(World*, Chunk*, MeshData*, int, int, int, Block);
using BlockCollideFunc = void(*)(GameState* state, World* world, vec3& delta, vec3 normal, Block block);

struct BlockAnimation
{
    vector<int> frames;
    float framesPerSec, timeLeft;
    int index;
};

struct BlockData
{
    uint16_t textures[6];
    bool invisible, passable, isFluid;
    BlockMeshType meshType;
    int cull;
    BuildBlockFunc buildFunc;
    BlockCollideFunc collideFunc;
    uint8_t alpha;
    int lightEmitted, lightStep;
    Sound onSetSound;
    BlockSurface surface;
    OverTimeDamage overTimeDamage;
    Color tint;
    char* name;
};

static int ComputeAnimationFrame(BlockAnimation& anim, float deltaTime);
