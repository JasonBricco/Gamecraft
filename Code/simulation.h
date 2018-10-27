//
// Jason Bricco
//

#define EPA_TOLERANCE 0.001f

// Provides information about which side of an entity collided.
enum CollisionFlags
{
    HIT_NONE = 0,
    HIT_UP = 1,
    HIT_DOWN = 2,
    HIT_OTHER = 4
};

struct CollisionInfo
{
    vec3 mtv;
    vec3 normal;
};

struct HitInfo
{
    bool hit;
    ivec3 hitPos;
    ivec3 adjPos;
    vec3 normal;
};

struct Collider
{
    // World position.
    vec3 pos;

    virtual vec3 Support(vec3 dir) = 0;
};

// Axis-aligned bounding box.
// 'min' and 'max' are in local space. 
struct AABB : Collider
{
    vec3 min;
    vec3 max;

    AABB(vec3 pos, vec3 min, vec3 max);
    inline vec3 Support(vec3 dir);
};

// Capsule collider for dynamic entities.
struct Capsule : Collider
{
    float r;
    float yBase;
    float yTop;

    Capsule() {}
    Capsule(float r, float h);
    
    inline vec3 Support(vec3 dir);
};

struct Player
{
    Camera* camera;
    Capsule collider;
    vec3 pos;
    vec3 velocity;
    float speed;
    float friction;
    uint8_t colFlags;
    bool flying, speedMode;
    bool spawned;

    // Stores a list of collisions the player must test against, 
    // sorted by distance from the player.
    vector<AABB> possibleCollides;

    Player() {}
};

AABB::AABB(vec3 p, vec3 minP, vec3 maxP)
{
    pos = p;
    min = minP;
    max = maxP;
}

Capsule::Capsule(float radius, float h)
{
    pos = vec3(0.0f);
    r = radius;
    yBase = 0.0f;
    yTop = h;
}

static void SpawnPlayer(Player* player, Rectf spawnBounds);
