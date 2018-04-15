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

struct Wall
{
	// Position of the wall on the axis being tested.
    float pos;

    // 'deltaA' is the movement along the axis being tested. The others are
    // the movement on the other axes.
    float deltaA, deltaB, deltaC;

    // Differences in position between the wall and entity being tested.
    // 'relA' is the difference on the axis being tested.
    float relA, relB, relC;
   
   	// Block boundaries on the axes not being tested, to ensure collision is
   	// confined to the boundaries of the block being tested.
    float minB, maxB, minC, maxC;
    
    vec3 normal;
    CollisionFlags flag;
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
	vec3 offset;
	vec3 size;
};

struct Player
{
	Camera* camera;
	vec3 pos;
	Collider col;
	vec3 velocity;
	float speed;
	float friction;
	uint8_t collisionFlags;
	bool flying;
};

static Player* NewPlayer(vec3 pos);

static void Move(World* world, Player* player, vec3 accel, float deltaTime);
static void Simulate(World* world, Player* player, float deltaTime);

static HitInfo GetVoxelHit(World* world);
static bool VoxelRaycast(World* world, Ray ray, float dist, vec3* result);
static float BlockRayIntersection(vec3 blockPos, Ray ray);
