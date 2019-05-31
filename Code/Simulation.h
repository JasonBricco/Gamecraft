//
// Gamecraft
//

#define WALL_EPSILON 0.001953125f

// Provides information about which side of an entity collided.
enum CollisionFlags
{
    HIT_NONE = 0,
    HIT_UP = 1,
    HIT_DOWN = 2,
    HIT_SIDES = 4
};

struct HitInfo
{
    bool hit;
    ivec3 hitPos;
    ivec3 adjPos;
    vec3 normal;
};

struct AABB
{
    // Position is the center of the AABB.
    // Radius is the distance from the center to an edge.
    vec3 pos;
    vec3 radius;
};

struct MinMaxAABB
{
    vec3 min;
    vec3 max;
};

enum MoveState
{
    MOVE_NORMAL,
    MOVE_FLYING,
    MOVE_SWIMMING
};

struct Player
{
    vec3 pos;
    vec3 velocity;
    float speed;
    float friction;
    uint8_t colFlags;
    bool spawned, suspended;

    MoveState moveState;

    // The surface the player is standing on.
    BlockSurface surface;

    vector<Block> upperOverlap;
    vector<Block> lowerOverlap;

    vector<AABB> possibleCollides;
};

static void SpawnPlayer(GameState* state, World* world, Player* player, Rectf spawnBound);
static bool OverlapsBlock(Player* player, int bX, int bY, int bZ);
