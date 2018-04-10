// Voxel Engine
// Jason Bricco

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
};

struct Ray
{
	vec3 origin;
	vec3 dir;
};

static Player* NewPlayer(vec3 pos);

// Test to see if two AABBs overlap.
static bool OverlapAABB(AABB a, AABB b);

inline AABB NewAABB(vec3 center, vec3 radius);
inline AABB MoveAABB(AABB a, vec3 amount);

// Casts a ray with unit direction 'dir' against an AABB with origin
// org, lower-back position lb and right-top position rt. 
static bool RaycastAABB(vec3 dir, vec3 org, vec3 lb, vec3 rt, float* t);

static bool WillCollide(World* world, AABB a, Ray ray);

static void Move(World* world, Player* player, vec3 accel, float deltaTime);
