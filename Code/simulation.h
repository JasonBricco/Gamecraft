// Voxel Engine
// Jason Bricco

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
	vec3 pos;
	vec3 velocity;
	float speed;
	float friction;
	uint8_t colFlags;
	bool flying, speedMode;
};

static Player* NewPlayer(float pMin, float pMax);

static float BlockRayIntersection(vec3 blockPos, Ray ray);
static bool VoxelRaycast(World* world, Ray ray, float dist, vec3* result);
static HitInfo GetVoxelHit(Renderer* rend, World* world);

// Updates the simplex for the three point (triangle) case. 'abc' must be in 
// counterclockwise winding order.
static void UpdateSimplex3(vec3& a, vec3& b, vec3& c, vec3& d, int& dim, vec3& search);

// Updates the simplex for the four point (tetrahedron) case. 'a' is the top of the 
// tetrahedron. 'bcd' is the base in counterclockwise winding order. We know the 
// origin is above 'bcd' and below 'a' before calling.
static bool UpdateSimplex4(vec3& a, vec3& b, vec3& c, vec3& d, int& dim, vec3& search);

// Expanding polytope algorithm for finding the minimum translation vector.
static CollisionInfo EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider* colA, Collider* colB);

// Returns true if two colliders are intersecting using the GJK algorithm. 
// 'info', if given, will return a minimum translation vector and collision 
// normal using EPA.
static bool Intersect(Collider* colA, Collider* colB, CollisionInfo* info);

inline void CameraFollow(Player* player);

static void Move(World* world, Player* player, vec3 accel, float deltaTime);
static void Simulate(Renderer* rend, World* world, Player* player, float deltaTime);
