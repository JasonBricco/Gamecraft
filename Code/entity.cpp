// Voxel Engine
// Jason Bricco
// Created April 3, 2018

struct Player
{
	Camera* camera;
	vec3 pos;
	AABB collider;
	float speed;
};

static Player* NewPlayer(vec3 pos)
{
	Player* player = Malloc(Player);
	player->camera = NewCamera(pos);
	player->pos = pos;
	player->collider = NewAABB(pos, 1.0f, 1.0f, 1.0f);
	player->speed = 25.0f;

	return player;
}

static bool WillCollide(World* world, AABB a)
{
	int bX = (int)round(a.center.x), bY = (int)round(a.center.y), bZ = (int)round(a.center.z);

	for (int y = -1; y <= 1; y++)
	{
		for (int z = -1; z <= 1; z++)
		{
			for (int x = -1; x <= 1; x++)
			{
				int block = GetBlock(world, bX + x, bY + y, bZ + z);
				
				if (block != 0 && TestBlock(a, x, y, z))
					return true;
			}
		}
	}

	return false;
}

static void Move(World* world, Player* player, vec3 amount)
{
	AABB test = MoveAABB(player->collider, amount);

	if (WillCollide(world, test))
		return;

	player->pos += amount;
	player->collider = test;
	player->camera->pos += amount;
	UpdateCameraVectors(player->camera);
}
