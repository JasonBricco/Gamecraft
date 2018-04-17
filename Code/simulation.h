// Voxel Engine
// Jason Bricco

#define EPA_TOLERANCE 0.0001f

// Provides information about which side of an entity collided.
enum CollisionFlags
{
	HIT_NONE = 0,
	HIT_UP = 1,
	HIT_DOWN = 2,
	HIT_OTHER = 4
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

// Axis-aligned bounding box. 'offset' is the offset from the world 
// position of the entity using this collider. 'min' and 'max' are
// in local space. 
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

	Capsule(float r, float h);
	inline vec3 Support(vec3 dir);
};


struct Player
{
	Camera* camera;
	Capsule collider;
	float colOffset;
	vec3 pos;
	vec3 velocity;
	float speed;
	float friction;
	uint8_t collisionFlags;
	bool flying;
};

static Player* NewPlayer(vec3 pos);

// Expanding polytope algorithm for finding the minimum translation vector.
static vec3 EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider* colA, Collider* colB);

// Updates the simplex for the three point (triangle) case. 'abc' must be in 
// counterclockwise winding order.
static void UpdateSimplex3(vec3& a, vec3& b, vec3& c, vec3& d, int& n, vec3& search);

// Updates the simplex for the four point (tetrahedron) case. 'a' is the top of the 
// tetrahedron. 'bcd' is the base in counterclockwise winding order. We know the 
// origin is above 'bcd' and below 'a' before calling.
static bool UpdateSimplex4(vec3& a, vec3& b, vec3& c, vec3& d, int& n, vec3& search);

// Returns the farthest point along an AABB in the given direction in world space.
static vec3 SupportAABB(vec3 pos, AABB bb, vec3 dir);

// Returns the farthest point along a capsule in the given direction in world space.
static vec3 SupportCapsule(vec3 pos, Capsule c, vec3 dir);

// Returns true if two colliders are intersecting. 'mtv' represents a
// minimum translation vector. If supplied, it can be used for 
// collision separation.
static bool Intersect(Collider* a, Collider* b, vec3* mtv);

static void Move(World* world, Player* player, vec3 accel, float deltaTime);
static void Simulate(World* world, Player* player, float deltaTime);

static HitInfo GetVoxelHit(World* world);
static bool VoxelRaycast(World* world, Ray ray, float dist, vec3* result);
static float BlockRayIntersection(vec3 blockPos, Ray ray);
