// Voxel Engine
// Jason Bricco

struct Player
{
	Camera* camera;
	vec3 pos;
	AABB collider;
	float speed;
};

static Player* NewPlayer(vec3 pos);
static bool WillCollide(World* world, AABB a);
static void Move(World* world, Player* player, vec3 amount);
