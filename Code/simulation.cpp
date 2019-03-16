//
// Jason Bricco
//

static inline AABB AABBFromCorner(vec3 corner, vec3 size)
{
	return { corner, size };
}

static inline AABB AABBFromCenter(vec3 center, vec3 size)
{
	vec3 corner = center - (size * 0.5f);
	return { corner, size };
}

static inline void ExpandAABB(AABB& bb, vec3 amount)
{
	bb.size += (amount * 2.0f);
	bb.pos -= amount;
}

static inline void ShrinkAABB(AABB& bb, vec3 amount)
{
	bb.size -= (amount * 2.0f);
	bb.pos += amount;
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

static void TestCollision(AABB a, AABB b, vec3 delta, float& tMin)
{
	ExpandAABB(b, a.size * 0.5f);
	ShrinkAABB(a, a.size * 0.5f);

	float epsilon = 0.0001f;
	vec3 wMin = b.pos, wMax = b.pos + b.size;

	// Test against top surface.
	if (delta.y != 0.0f)
	{
		float tResult = (wMax.y - a.pos.y) / delta.y;
		float x = a.pos.x + tResult * delta.x;
		float z = a.pos.z + tResult * delta.z;

		if (tResult > 0.0f && tResult < tMin)
		{
			if (x >= wMin.x && x <= wMax.x && z >= wMin.z && z <= wMax.z)
				tMin = Max(0.0f, tResult - epsilon);
		}
	}

	// Test against bottom surface.
	if (delta.y != 0.0f)
	{
		float tResult = (wMin.y - a.pos.y) / delta.y;
		float x = a.pos.x + tResult * delta.x;
		float z = a.pos.z + tResult * delta.z;

		if (tResult > 0.0f && tResult < tMin)
		{
			if (x >= wMin.x && x <= wMax.x && z >= wMin.z && z <= wMax.z)
				tMin = Max(0.0f, tResult - epsilon);
		}
	}

	// Test against left wall.
	if (delta.x != 0.0f)
	{
		float tResult = (wMin.x - a.pos.x) / delta.x;
		float y = a.pos.y + tResult * delta.y;
		float z = a.pos.z + tResult * delta.z;

		if (tResult > 0.0f && tResult < tMin)
		{
			if (y >= wMin.y && y <= wMax.y && z >= wMin.z && z <= wMax.z)
				tMin = Max(0.0f, tResult - epsilon);
		}
	}

	// Test against right wall.
	if (delta.x != 0.0f)
	{
		float tResult = (wMax.x - a.pos.x) / delta.x;
		float y = a.pos.y + tResult * delta.y;
		float z = a.pos.z + tResult * delta.z;

		if (tResult > 0.0f && tResult < tMin)
		{
			if (y >= wMin.y && y <= wMax.y && z >= wMin.z && z <= wMax.z)
				tMin = Max(0.0f, tResult - epsilon);
		}
	}

	// Test against front wall.
	if (delta.z != 0.0f)
	{
		float tResult = (wMax.z - a.pos.z) / delta.z;
		float y = a.pos.y + tResult * delta.y;
		float x = a.pos.x + tResult * delta.x;

		if (tResult > 0.0f && tResult < tMin)
		{
			if (y >= wMin.y && y <= wMax.y && x >= wMin.x && x <= wMax.x)
				tMin = Max(0.0f, tResult - epsilon);
		}
	}

	// Test against back wall.
	if (delta.z != 0.0f)
	{
		float tResult = (wMin.z - a.pos.z) / delta.z;
		float y = a.pos.y + tResult * delta.y;
		float x = a.pos.x + tResult * delta.x;

		if (tResult > 0.0f && tResult < tMin)
		{
			if (y >= wMin.y && y <= wMax.y && x >= wMin.x && x <= wMax.x)
				tMin = Max(0.0f, tResult - epsilon);
		}
	}
}

static void Move(World* world, Player* player, vec3 accel, float deltaTime)
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

	vec3 target = player->pos + delta;

	AABB playerBB = AABBFromCenter(player->pos, vec3(0.6f, 1.8f, 0.6f));

	player->colFlags = HIT_NONE;

	// Player size in blocks.
	ivec3 bSize = CeilToInt(playerBB.size);

	LWorldPos start = BlockPos(player->pos);
	LWorldPos end = BlockPos(target);

	vec3 inf = vec3(INFINITY);

	if (distance2(GetV3(start), inf) > distance2(GetV3(end), inf))
	{
		ivec3 tmp = end;
		end = start;
		start = end;
	}

	// Compute the range of blocks we could touch with our movement. We'll test for collisions
	// with the blocks in this range.
	ivec3 min = start - bSize;
	ivec3 max = end + bSize;

	for (int z = min.z; z <= max.z; z++)
	{
		for (int y = min.y; y <= max.y; y++)
		{
			for (int x = min.x; x <= max.x; x++)
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

	float tMin = 1.0f;

	for (int i = 0; i < player->possibleCollides.size(); i++)
	{
		AABB bb = player->possibleCollides[i];
		TestCollision(playerBB, bb, delta, tMin);
 	}

 	player->pos += delta * tMin;
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

	Move(world, player, accel, deltaTime);

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
