// Voxel Engine
// Jason Bricco

static Player* NewPlayer(vec3 pos)
{
	Player* player = Malloc(Player);
	player->camera = NewCamera(pos);
	player->pos = pos;
	player->collider = NewAABB(pos, vec3(0.5f, 0.9f, 0.5f));
	player->speed = 200.0f;
	player->friction = -8.0f;
	player->collisionFlags = HIT_NONE;

	return player;
}

inline AABB NewAABB(vec3 center, vec3 radius)
{
	return { center, radius };
}

static bool OverlapAABB(AABB a, AABB b)
{
	if (abs(a.c[0] - b.c[0]) > (a.r[0] + b.r[0])) return false;
	if (abs(a.c[1] - b.c[1]) > (a.r[1] + b.r[1])) return false;
	if (abs(a.c[2] - b.c[2]) > (a.r[2] + b.r[2])) return false;

	return true;
}

inline AABB MoveAABB(AABB a, vec3 amount)
{
	return NewAABB(a.c + amount, a.r);
}

static void Move(World* world, Player* player, vec3 accel, float deltaTime)
{
	accel = accel * player->speed;
	accel = accel + player->velocity * player->friction;

	// Using the following equations of motion:

	// - p' = 1/2at^2 + vt + p.
	// - v' = at + v.
	// - a = specified by input.

	// Where a = acceleration, v = velocity, and p = position.
	// v' and p' denote new versions, while non-prime denotes old.

	// These are found by integrating up from acceleration to velocity. Use derivation
	// to go from position down to velocity and then down to acceleration to see how 
	// we can integrate back up.
	vec3 delta = accel * 0.5f * Square(deltaTime) + player->velocity * deltaTime;
	player->velocity = accel * deltaTime + player->velocity;

	// Position to try to move to.
	vec3 target = player->pos + delta;

	AABB a = player->collider;
	AABB test = MoveAABB(a, delta);

	int bX = (int)round(a.c.x), bY = (int)round(a.c.y), bZ = (int)round(a.c.z);

	for (int y = -1; y <= 1; y++)
	{
		for (int z = -1; z <= 1; z++)
		{
			for (int x = -1; x <= 1; x++)
			{
				int block = GetBlock(world, bX + x, bY + y, bZ + z);

				if (block != 0)
				{
					vec3 bPos(bX, bY, bZ);
					AABB b = NewAABB(bPos, vec3(0.5f));
					
					if (OverlapAABB(a, b))
						return;
				}
			}
		}
	}

	player->pos += delta;
	player->camera->pos += delta;
	player->collider = test;
	UpdateCameraVectors(player->camera);
}
