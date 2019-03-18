//
// Jason Bricco
//

static inline AABB AABBFromCorner(vec3 corner, vec3 size)
{
	vec3 radius = size * 0.5f;
	return { corner + radius, radius };
}

static inline AABB AABBFromCenter(vec3 center, vec3 size)
{
	return { center, size * 0.5f };
}

static inline void ExpandAABB(AABB& bb, vec3 amount)
{
	bb.radius += amount;
}

static inline void ShrinkAABB(AABB& bb, vec3 amount)
{
	bb.radius -= amount;
}

static float BlockRayIntersection(vec3 blockPos, Ray ray)
{
	float nearP = -FLT_MAX;
	float farP = FLT_MAX;
	
	for (int i = 0; i < 3; i++) 
	{
		float min = blockPos[i] - 0.5f;
		float max = blockPos[i] + 0.5f;
		
		float pos = ray.origin[i];
		float dir = ray.dir[i];
		
		if (abs(dir) <= EPSILON) 
		{
			if ((pos < min) || (pos > max)) 
				return FLT_MAX;
		}
		
		float t0 = (min - pos) / dir;
		float t1 = (max - pos) / dir;
		
		if (t0 > t1) 
		{
			float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
		
		nearP = Max(t0, nearP);
		farP = Min(t1, farP);
		
		if (nearP > farP) return FLT_MAX;
		if (farP < 0.0f) return FLT_MAX;
	}
	
	return nearP > 0.0f ? nearP : farP;
}

static bool VoxelRaycast(World* world, Ray ray, float dist, vec3* result)
{
	ivec3 start = BlockPos(ray.origin);
	ivec3 end = BlockPos(ray.origin + ray.dir * dist);

	if (start.x > end.x)
	{
		int tmp = start.x;
		start.x = end.x;
		end.x = tmp;
	}
	
	if (start.y > end.y) 
	{
		int tmp = start.y;
		start.y = end.y;
		end.y = tmp;
	}
	
	if (start.z > end.z) 
	{
		int tmp = start.z;
		start.z = end.z;
		end.z = tmp;
	}

	float minDistance = dist;

	for (int z = start.z; z <= end.z; z++) 
	{
		for (int y = start.y; y <= end.y; y++) 
		{
			for (int x = start.x; x <= end.x; x++) 
			{
				Block block = GetBlock(world, x, y, z);

				if (IsPassable(world, block)) continue;

				float newDist = BlockRayIntersection(vec3((float)x, (float)y, (float)z), ray);
				minDistance = Min(minDistance, newDist);
			}
		}
	}

	if (minDistance != dist)
	{
		*result = ray.origin + ray.dir * minDistance;
		return true;
	}

	return false;
}

static HitInfo GetVoxelHit(GameState* state, Camera* cam, World* world)
{
	HitInfo info = {};
	Ray ray = ScreenCenterToRay(state, cam);

	vec3 point;
	
	if (VoxelRaycast(world, ray, 15.0f, &point))
	{
		info.hit = true;
		info.hitPos = BlockPos(point + ray.dir * 0.01f);
		point = point - (ray.dir * 0.01f);
		info.adjPos = BlockPos(point);
		info.normal = GetV3(info.hitPos - info.adjPos);
	}

	return info;
}

static inline bool TestWall(vec3 delta, vec3 p, float wallP, vec3 wMin, vec3 wMax, int axis0, int axis1, int axis2, float& tMin)
{
	if (delta[axis0] != 0.0f)
	{
		float tResult = (wallP - p[axis0]) / delta[axis0];

		if (tResult > 0.0f && tResult < tMin)
		{
			float o1 = p[axis1] + tResult * delta[axis1];
			float o2 = p[axis2] + tResult * delta[axis2];

			if (o1 >= wMin[axis1] && o1 <= wMax[axis1] && o2 >= wMin[axis2] && o2 <= wMax[axis2])
			{
				tMin = tResult;
				return true;
			}
		}
	}

	return false;
}

static void TestCollision(World* world, AABB a, AABB b, vec3 delta, float& tMin, vec3& normal)
{
	ExpandAABB(b, a.radius);
	ShrinkAABB(a, a.radius);

	ivec3 bPos = BlockPos(b.pos);
	vec3 wMin = b.pos - b.radius, wMax = b.pos + b.radius;

	bool upPassable = IsPassable(world, GetBlock(world, bPos.x, bPos.y + 1, bPos.z));
	bool downPassable = IsPassable(world, GetBlock(world, bPos.x, bPos.y - 1, bPos.z));
	bool leftPassable = IsPassable(world, GetBlock(world, bPos.x - 1, bPos.y, bPos.z));
	bool rightPassable = IsPassable(world, GetBlock(world, bPos.x + 1, bPos.y, bPos.z));
	bool frontPassable = IsPassable(world, GetBlock(world, bPos.x, bPos.y, bPos.z + 1));
	bool backPassable = IsPassable(world, GetBlock(world, bPos.x, bPos.y, bPos.z - 1));

	// Top surface.
	if (upPassable && TestWall(delta, a.pos, wMax.y, wMin, wMax, 1, 0, 2, tMin))
		normal = vec3(0.0f, 1.0f, 0.0f);

	// Bottom surface.
	if (downPassable && TestWall(delta, a.pos, wMin.y, wMin, wMax, 1, 0, 2, tMin))
		normal = vec3(0.0f, -1.0f, 0.0f);

	// Left wall.
	if (leftPassable && TestWall(delta, a.pos, wMin.x, wMin, wMax, 0, 1, 2, tMin))
		normal = vec3(-1.0f, 0.0f, 0.0f);

	// Right wall.
	if (rightPassable && TestWall(delta, a.pos, wMax.x, wMin, wMax, 0, 1, 2, tMin))
		normal = vec3(1.0f, 0.0f, 0.0f);

	// Front wall.
	if (frontPassable && TestWall(delta, a.pos, wMax.z, wMin, wMax, 2, 0, 1, tMin))
		normal = vec3(0.0f, 0.0f, 1.0f);

	// Back wall.
	if (backPassable && TestWall(delta, a.pos, wMin.z, wMin, wMax, 2, 0, 1, tMin))
		normal = vec3(0.0f, 0.0f, -1.0f);
}

static void Move(Input& input, World* world, Player* player, vec3 accel, float deltaTime)
{
	accel = accel * player->speed;
	accel = accel + ((player->velocity * player->friction));

	// Gravity.
	if (!player->flying) accel.y = -30.0f;

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

	static vec3 savedPos;

	if (KeyPressed(input, KEY_F1))
		delta = vec3(5.0f, -3.0f, 0.0f);

	if (KeyPressed(input, KEY_F2))
		savedPos = player->pos;

	if (KeyPressed(input, KEY_F4))
	{
		player->pos = savedPos;
		player->velocity = vec3(0.0f);
		delta = vec3(0.0f);
	}

	vec3 target = player->pos + delta;

	AABB playerBB = AABBFromCenter(player->pos, vec3(0.6f, 1.8f, 0.6f));

	player->colFlags = HIT_NONE;

	// Player size in blocks.
	ivec3 bSize = CeilToInt(playerBB.radius * 2.0f);

	LWorldPos start = BlockPos(player->pos);
	LWorldPos end = BlockPos(target);

	// Compute the range of blocks we could touch with our movement. We'll test for collisions
	// with the blocks in this range.
	int minX = Min(start.x, end.x) - bSize.x;
	int minY = Min(start.y, end.y) - bSize.y;
	int minZ = Min(start.z, end.z) - bSize.z;

	int maxX = Max(start.x, end.x) + bSize.x;
	int maxY = Max(start.y, end.y) + bSize.y;
	int maxZ = Max(start.z, end.z) + bSize.z;

	for (int z = minZ; z <= maxZ; z++)
	{
		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				Block block = GetBlock(world, x, y, z);

				if (!IsPassable(world, block))
				{
					AABB bb = AABBFromCenter(vec3(x, y, z), vec3(1.0f));
					player->possibleCollides.push_back(bb);
				}
			}
		}
	}

	sort(player->possibleCollides.begin(), player->possibleCollides.end(), [player](auto a, auto b) 
    { 
        float distA = distance2(vec3(a.pos.x + 0.5f, a.pos.y + 0.5f, a.pos.z + 0.5f), player->pos);
        float distB = distance2(vec3(b.pos.x + 0.5f, b.pos.y + 0.5f, b.pos.z + 0.5f), player->pos);
		return distA < distB;
    });

	float tRemaining = 1.0f;

	for (int it = 0; it < 4 && tRemaining > 0.0f; it++)
	{
		float tMin = 1.0f;
		vec3 normal = vec3(0.0f);

		for (int i = 0; i < player->possibleCollides.size(); i++)
		{
			AABB bb = player->possibleCollides[i];
			TestCollision(world, playerBB, bb, delta, tMin, normal);
	 	}

	 	player->pos += delta * tMin + (normal * 0.001f);

	 	// Subtract away the component of the velocity that collides with the wall and leave the
	 	// remaining velocity intact.
	 	player->velocity -= dot(player->velocity, normal) * normal;
	 	delta -= dot(delta, normal) * normal;

	 	tRemaining -= tMin;
	}

    player->possibleCollides.clear();
}

static bool OverlapsBlock(Player*, int, int, int)
{
	return false;
}

static void Simulate(GameState* state, World* world, Player* player, float deltaTime)
{
	Input& input = state->input;
	Camera* cam = state->camera;

	vec3 accel = vec3(0.0f);

	if (KeyHeld(input, KEY_UP)) accel += GetXZ(cam->forward);
	if (KeyHeld(input, KEY_DOWN)) accel += GetXZ(-cam->forward);
	if (KeyHeld(input, KEY_LEFT)) accel += GetXZ(-cam->right);
	if (KeyHeld(input, KEY_RIGHT)) accel += GetXZ(cam->right);

	if (accel != vec3(0.0f))
		accel = normalize(accel);

	if (KeyPressed(input, KEY_TAB))
		player->flying = !player->flying;

	if (KeyPressed(input, KEY_P))
		player->speedMode = !player->speedMode;

	if (player->flying)
	{
		player->speed = player->speedMode ? 1000.0f : 200.0f;

		if (KeyHeld(input, KEY_SPACE))
			accel.y = 1.0f;

		if (KeyHeld(input, KEY_SHIFT)) accel.y = -1.0f;
	}
	else 
	{
		player->speed = 50.0f;

		if ((player->colFlags & HIT_DOWN) && KeyHeld(input, KEY_SPACE))
			player->velocity.y = 10.0f;
	}

	Move(input, world, player, accel, deltaTime);

	float min = 0.0f, max = (float)(world->size * CHUNK_SIZE_H - 1);
	player->pos.x = std::clamp(player->pos.x, min, max);
	player->pos.z = std::clamp(player->pos.z, min, max);

	MoveCamera(cam, player->pos);

	// Tint the screen if the camera is in water. Subtract 0.1 to account
	// for the near clip plane of the camera.
	vec3 eyePos = vec3(cam->pos.x, cam->pos.y - 0.1f, cam->pos.z);
	Block eyeBlock = GetBlock(world, BlockPos(eyePos));

	if (eyeBlock == BLOCK_WATER)
	{
		cam->fadeColor = vec4(0.17f, 0.45f, 0.69f, 0.75f);
		cam->disableFluidCull = true;
	}
	else
	{
		cam->fadeColor = CLEAR_COLOR;
		cam->disableFluidCull = false;
	}

	int op = MousePressed(input, 0) ? 0 : MousePressed(input, 1) ? 1 : -1;

	world->cursorOnBlock = false;
	HitInfo info = GetVoxelHit(state, cam, world);

	if (info.hit)
	{
		world->cursorOnBlock = true;
		world->cursorBlockPos = info.hitPos;

		ivec3 setPos;

		if (op == 0)
		{
			setPos = info.adjPos;
			SetBlock(world, setPos, world->blockToSet);
		}
		else if (op == 1)
		{
			setPos = info.hitPos;
			SetBlock(world, setPos, BLOCK_AIR);
		}
	}
}

// Creates and spawns the player. The player is spawned within the center local space chunk.
static Player* NewPlayer()
{
	Player* player = CallocStruct(Player);
	Construct(player, Player);
	
	player->velocity = vec3(0.0f);
	player->speed = 50.0f;
	player->friction = -8.0f;
	player->colFlags = HIT_NONE;
	player->flying = false;
	player->speedMode = false;
	player->spawned = false;
	return player;
}

static void SpawnPlayer(GameState* state, World* world, Player* player, Rectf spawnBounds)
{
	vec3 spawn = spawnBounds.min + (CHUNK_SIZE_H / 2.0f);

	Ray ray = { vec3(spawn.x, WORLD_BLOCK_HEIGHT, spawn.z), vec3(0.0, -1.0f, 0.0f) };

	vec3 loc;
	
	if (VoxelRaycast(world, ray, WORLD_BLOCK_HEIGHT, &loc))
		spawn.y = loc.y + 2.0f;
	else spawn.y = 258.0f;
	
	player->pos = spawn;

	MoveCamera(state->camera, player->pos);
	player->spawned = true;
}
