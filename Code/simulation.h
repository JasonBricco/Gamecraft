// Voxel Engine
// Jason Bricco

// Provides information about which side of an entity collided.
enum CollisionFlags
{
	HIT_NONE = 0,
	HIT_UP = 1,
	HIT_DOWN = 2,
	HIT_OTHER = 4
};

// Axis-aligned bounding box with center c and radius r.
struct AABB
{
	vec3 c;
	vec3 r;
};

struct Player
{
	Camera* camera;
	vec3 pos;
	AABB collider;
	vec3 velocity;
	float speed;
	float friction;
	uint8_t collisionFlags;
};

static Player* NewPlayer(vec3 pos);

// Test to see if two AABBs overlap.
static bool OverlapAABB(AABB a, AABB b);

inline AABB NewAABB(vec3 center, vec3 radius);
inline AABB MoveAABB(AABB a, vec3 amount);

static void Move(World* world, Player* player, vec3 accel, float deltaTime);
