//
// Gamecraft
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

static inline MinMaxAABB MinMaxAABBFromCorner(vec3 corner, vec3 size)
{
	return { corner, corner + size };
}

static inline MinMaxAABB MinMaxAABBFromCenter(vec3 center, vec3 radius)
{
	vec3 min = center - radius;
	vec3 max = center + radius;
	return { min, max };
}

static inline void ExpandAABB(AABB& bb, vec3 amount)
{
	bb.radius += amount;
}

static inline void ShrinkAABB(AABB& bb, vec3 amount)
{
	bb.radius -= amount;
}

static inline AABB GetPlayerAABB(Player* player)
{
	return AABBFromCenter(player->pos, vec3(0.6f, 1.8f, 0.6f));
}

static inline void GetLowerUpperAABB(Player* player, AABB& lower, AABB& upper)
{
	lower = AABBFromCenter(player->pos - 0.45f, vec3(0.6f, 0.9f, 0.6f));
	upper = AABBFromCenter(player->pos + 0.45f, vec3(0.6f, 0.9f, 0.6f));
}

static inline MinMaxAABB GetMinMaxAABB(AABB bb)
{
	return MinMaxAABBFromCenter(bb.center, bb.radius);
}

static inline bool OverlapAABB(MinMaxAABB a, MinMaxAABB b)
{
	bool overlapX = a.min.x <= b.max.x && a.max.x >= b.min.x;
	bool overlapY = a.min.y <= b.max.y && a.max.y >= b.min.y;
	bool overlapZ = a.min.z <= b.max.z && a.max.z >= b.min.z;

	return overlapX && overlapY && overlapZ;
}

static inline bool OverlapAABB(AABB a, AABB b)
{
	MinMaxAABB mmA = GetMinMaxAABB(a);
	MinMaxAABB mmB = GetMinMaxAABB(b);

	return OverlapAABB(mmA, mmB);
}

static float BlockRayIntersection(vec3 blockPos, Ray ray)
{
	float nearP = -FLT_MAX;
	float farP = FLT_MAX;
	
	for (int i = 0; i < 3; i++) 
	{
		float min = blockPos[i];
		float max = blockPos[i] + 1.0f;
		
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
		point -= (ray.dir * 0.01f);
		info.adjPos = BlockPos(point);
		info.normal = GetV3(info.hitPos - info.adjPos);
	}

	return info;
}

static void TeleportPlayerCallback(GameState* state, World* world)
{
	WorldLocation p = state->teleportLoc;

	ChunkP cP = WorldToChunkP(p.wP);
	UpdateWorldRef(world, cP);

	LWorldP lwP = ChunkToLWorldP(world, cP, p.rP);

	Player* player = world->player;
	player->suspended = true;
	player->pos = vec3(lwP.x + 0.5f, lwP.y + 1.0f, lwP.z + 0.5f);

	MoveCamera(state->camera, player->pos);
	UpdateCameraVectors(state->camera);

	ShiftWorld(state, world);
}

static void TeleportPlayer(GameState* state, WorldLocation p)
{
	state->teleportLoc = p;
	BeginLoadingScreen(state, 0.5f, TeleportPlayerCallback);
}

static void KillPlayer(GameState* state, Player* player)
{
	SetScreenFade(state->renderer, player->damageFade, FADE_PRIORITY_DAMAGE);
	state->renderer.fadeTimeLeft = FLT_MAX;
	Pause(state, PLAYER_DEAD);
}

static void ApplyDamage(GameState* state, Player* player, int amount)
{
	player->health = Max(player->health - amount, 0);
	FadeScreenForTime(state->renderer, player->damageFade, 0.1f, FADE_PRIORITY_DAMAGE);

	if (player->health <= 0) KillPlayer(state, player);
}

static inline bool TestWall(vec3 delta, vec3 p, float wallP, vec3 wMin, vec3 wMax, int axis0, int axis1, int axis2, float& tMin)
{
	if (delta[axis0] != 0.0f)
	{
		float tResult = (wallP - p[axis0]) / delta[axis0];

		if (tResult >= 0.0f && tResult < tMin)
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

static inline void TestCollision(World* world, Player* player, AABB a, AABB b, vec3 delta, float& tMin, vec3& normal, Block& collideBlock)
{
	ExpandAABB(b, a.radius);
	ShrinkAABB(a, a.radius);

	ivec3 bPos = BlockPos(b.center);
	vec3 wMin = b.center - b.radius, wMax = b.center + b.radius;

	Block up = GetBlock(world, bPos.x, bPos.y + 1, bPos.z);
	Block down = GetBlock(world, bPos.x, bPos.y - 1, bPos.z);
	Block left = GetBlock(world, bPos.x - 1, bPos.y, bPos.z);
	Block right = GetBlock(world, bPos.x + 1, bPos.y, bPos.z);
	Block front = GetBlock(world, bPos.x, bPos.y, bPos.z + 1);
	Block back = GetBlock(world, bPos.x, bPos.y, bPos.z - 1);

	// Top surface.
	if (IsPassable(world, up) && TestWall(delta, a.center, wMax.y, wMin, wMax, 1, 0, 2, tMin))
	{
		normal = vec3(0.0f, 1.0f, 0.0f);
		
		if (delta.y < 0.0f)
		{
			player->colFlags |= HIT_DOWN;
			Block block = GetBlock(world, bPos);
			player->surface = GetSurface(world, block);
			collideBlock = block;
		}
	}

	// Bottom surface.
	if (IsPassable(world, down) && TestWall(delta, a.center, wMin.y, wMin, wMax, 1, 0, 2, tMin))
	{
		normal = vec3(0.0f, -1.0f, 0.0f);
		player->colFlags |= HIT_UP;
		collideBlock = GetBlock(world, bPos);
	}

	// Left wall.
	if (IsPassable(world, left) && TestWall(delta, a.center, wMin.x, wMin, wMax, 0, 1, 2, tMin))
	{
		normal = vec3(-1.0f, 0.0f, 0.0f);
		player->colFlags |= HIT_SIDES;
		collideBlock = GetBlock(world, bPos);
	}

	// Right wall.
	if (IsPassable(world, right) && TestWall(delta, a.center, wMax.x, wMin, wMax, 0, 1, 2, tMin))
	{
		normal = vec3(1.0f, 0.0f, 0.0f);
		player->colFlags |= HIT_SIDES;
		collideBlock = GetBlock(world, bPos);
	}

	// Front wall.
	if (IsPassable(world, front) && TestWall(delta, a.center, wMax.z, wMin, wMax, 2, 0, 1, tMin))
	{
		normal = vec3(0.0f, 0.0f, 1.0f);
		player->colFlags |= HIT_SIDES;
		collideBlock = GetBlock(world, bPos);
	}

	// Back wall.
	if (IsPassable(world, back) && TestWall(delta, a.center, wMin.z, wMin, wMax, 2, 0, 1, tMin))
	{
		normal = vec3(0.0f, 0.0f, -1.0f);
		player->colFlags |= HIT_SIDES;
		collideBlock = GetBlock(world, bPos);
	}
}

static void ApplyBlockSurface(Player* player, vec3 accel, float deltaTime)
{
	switch (player->surface)
	{
		case SURFACE_ICE:
		{
			player->velocity.x = (accel.x * 0.1f) * deltaTime + player->velocity.x;
			player->velocity.y = accel.y * deltaTime + player->velocity.y;
			player->velocity.z = (accel.z * 0.1f) * deltaTime + player->velocity.z;
		} break;

		case SURFACE_WATER:
			player->velocity = (accel * 0.35f) * deltaTime + player->velocity;
			break;

		default:
			player->velocity = accel * deltaTime + player->velocity;
	}

	player->velocity.y = Max(player->velocity.y, -100.0f);
}

static void MovePlayer(GameState* state, World* world, vec3 accel, float deltaTime, float gravity)
{
	TIMED_FUNCTION;

	Player* player = world->player;

	accel *= player->speed;
	accel += player->velocity * player->friction;

	if (gravity != 0.0f)
		accel.y = gravity;

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
	ApplyBlockSurface(player, accel, deltaTime);
	
	vec3 target = player->pos + delta;
	AABB playerBB = GetPlayerAABB(player);

	player->colFlags = HIT_NONE;

	// Player size in blocks.
	ivec3 bSize = CeilToInt(playerBB.radius * 2.0f);

	LWorldP start = BlockPos(player->pos);
	LWorldP end = BlockPos(target);

	// Compute the range of blocks we could touch with our movement. We'll test for collisions
	// with the blocks in this range.
	int minX = Min(start.x, end.x) - bSize.x;
	int minY = Min(start.y, end.y) - bSize.y;
	int minZ = Min(start.z, end.z) - bSize.z;

	int maxX = Max(start.x, end.x) + bSize.x;
	int maxY = Max(start.y, end.y) + bSize.y;
	int maxZ = Max(start.z, end.z) + bSize.z;

	AABB lower, upper;
	GetLowerUpperAABB(player, lower, upper);

	player->lowerOverlap.clear();
	player->upperOverlap.clear();

	for (int z = minZ; z <= maxZ; z++)
	{
		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				Block block = GetBlock(world, x, y, z);

				if (!IsPassable(world, block))
				{
					AABB bb = AABBFromCorner(vec3(x, y, z), vec3(1.0f));
					player->possibleCollides.push_back(bb);
				}
				else if (block != BLOCK_AIR)
				{
					AABB bb = AABBFromCorner(vec3(x, y, z), vec3(1.0f));

					if (OverlapAABB(bb, lower))
						player->lowerOverlap.push_back(block);

					if (OverlapAABB(bb, upper))
						player->upperOverlap.push_back(block);
				}
			}
		}
	}

	sort(player->possibleCollides.begin(), player->possibleCollides.end(), [player](auto a, auto b) 
    { 
        float distA = distance2(a.center, player->pos);
        float distB = distance2(b.center, player->pos);
		return distA < distB;
    });

	float tRemaining = 1.0f;

	for (int it = 0; it < 3 && tRemaining > 0.0f; it++)
	{
		float tMin = 1.0f;
		vec3 normal = vec3(0.0f);

		Block collideBlock = BLOCK_AIR;

		for (int i = 0; i < player->possibleCollides.size(); i++)
		{
			AABB bb = player->possibleCollides[i];
			TestCollision(world, player, playerBB, bb, delta, tMin, normal, collideBlock);
	 	}

	 	player->pos += delta * tMin;

	 	if (normal != vec3(0.0f))
	 	{
	 		vec3 nOffset = (normal * WALL_EPSILON);
	 		vec3 dOffset = (normalize(delta) * -WALL_EPSILON);

	 		float offX = fabs(nOffset.x) > fabs(dOffset.x) ? nOffset.x : dOffset.x;
	 		float offY = fabs(nOffset.y) > fabs(dOffset.y) ? nOffset.y : dOffset.y;
	 		float offZ = fabs(nOffset.z) > fabs(dOffset.z) ? nOffset.z : dOffset.z;

	 		player->pos += vec3(offX, offY, offZ);
	 	}

	 	playerBB = GetPlayerAABB(player);

	 	GetCollideFunc(world, collideBlock)(state, world, delta, normal, collideBlock);

	 	delta -= (delta * tMin);
	 	tRemaining -= (tMin * tRemaining);
	}

	if (!(player->colFlags & HIT_DOWN))
		player->surface = SURFACE_NORMAL;

    player->possibleCollides.clear();
}

static void IgnoreCollideFunc(GameState*, World*, vec3&, vec3, Block) {}

static void BounceCollideFunc(GameState*, World* world, vec3& delta, vec3 normal, Block)
{
	Player* player = world->player;
	player->velocity -= 2 * dot(player->velocity, normal) * normal * 0.9f;
	delta -= 2 * dot(delta, normal) * normal * 0.9f;
}

static void DefaultCollideFunc(GameState*, World* world, vec3& delta, vec3 normal, Block)
{
	Player* player = world->player;

	// Subtract away the component of the velocity that collides with the wall and leave the
 	// remaining velocity intact.
 	player->velocity -= dot(player->velocity, normal) * normal;

 	delta -= dot(delta, normal) * normal;
}

static void OverTimeDamageCollideFunc(GameState* state, World* world, vec3& delta, vec3 normal, Block block)
{
	DefaultCollideFunc(state, world, delta, normal, block);

	bool doApply = true;
	Player* player = world->player;

	for (OverTimeDamage& ot : player->overTimeDamage)
	{
		if (ot.type == block)
		{
			ot.active = true;
			doApply = false;
			break;
		}
	}

	if (doApply)
	{
		OverTimeDamage ot = GetOverTimeDamage(world, block);
		ot.active = true;
		player->overTimeDamage.push_back(ot);
	}
}

static void KillCollideFunc(GameState* state, World* world, vec3&, vec3, Block)
{
	Player* player = world->player;
	player->velocity = vec3(0.0f);
	ApplyDamage(state, player, INT_MAX);
}

static bool OverlapsBlock(Player* player, int x, int y, int z)
{
	AABB playerBB = GetPlayerAABB(player);
	AABB block = AABBFromCorner(vec3(x, y, z), vec3(1.0f));
	return OverlapAABB(playerBB, block);
}

static void SetBlockFromInput(World* world, ivec3 pos, Input& input, Block block)
{
	SetBlock(world, pos, block);
	input.blockSetDelay = 0.15f;
}

static void HandleEditInput(GameState* state, Input& input, World* world, float deltaTime)
{
	input.blockSetDelay -= deltaTime;

	world->cursorOnBlock = false;
	HitInfo info = GetVoxelHit(state, state->camera, world);

	if (info.hit)
	{
		world->cursorOnBlock = true;
		world->cursorBlockPos = info.hitPos;
		world->adjBlockPos = info.adjPos;

		if (KeyPressed(input, KEY_R))
			world->blockToSet = (BlockType)GetBlock(world, world->cursorBlockPos);

		if (MousePressed(input, 0))
			SetBlockFromInput(world, info.adjPos, input, world->blockToSet);
		else if (MousePressed(input, 1))
			SetBlockFromInput(world, info.hitPos, input, BLOCK_AIR);
		else if (input.blockSetDelay < 0.0f)
		{	
			if (MouseHeld(input, 0))
				SetBlockFromInput(world, info.adjPos, input, world->blockToSet);
			else if (MouseHeld(input, 1))
				SetBlockFromInput(world, info.hitPos, input, BLOCK_AIR);
		}
	}
}

static inline bool ListContainsFluid(vector<Block>& list, Block block)
{
	for (Block next : list)
	{
		if (next == block)
			return true;
	}

	return false;
}

static void ApplyOverTimeDamage(GameState* state, Player* player, float deltaTime)
{
	auto& otList = player->overTimeDamage;

	// Apply over time damage effects.
	for (auto it = otList.begin(); it != otList.end();)
	{
		it->timeLeft -= deltaTime;

		if (it->timeLeft <= 0.0f)
		{
			if (!it->active)
			{
				it = otList.erase(it);
				continue;
			}
	
			ApplyDamage(state, player, it->damage);
			it->timeLeft = it->interval;
		}

		it++;
	}
}

static inline void GetPlayerInsideFluid(World* world, bool& lower, bool& upper)
{
	auto isFluid = [world](Block block) {  return IsFluid(world, block); };

	Player* player = world->player;
	lower = find_if(player->lowerOverlap.begin(), player->lowerOverlap.end(), isFluid) != player->lowerOverlap.end();
	upper = find_if(player->upperOverlap.begin(), player->upperOverlap.end(), isFluid) != player->upperOverlap.end();
}

static void SetMoveParams(World* world, Player* player, Input& input, vec3& accel, float& gravity)
{
	gravity = -30.0f;

	switch (player->moveState)
	{
		case MOVE_NORMAL:
		{
			accel.y = 0.0f;
			player->speed = player->walkSpeed;
			player->friction = -8.0f;

			if ((player->colFlags & HIT_DOWN) && KeyHeld(input, KEY_SPACE))
				player->velocity.y = player->jumpVelocity;

			bool lower, upper;
			GetPlayerInsideFluid(world, lower, upper);

			if (lower || upper)
				player->moveState = MOVE_SWIMMING;
		} break;

		case MOVE_FLYING:
		{ 
			player->speed = player->flySpeed;
			player->friction = -8.0f;
			gravity = 0.0f;
			accel.y = 0.0f;

			// Gravity subtracts 30 from acceleration. These values will
			// ensure it becomes 1 or -1 when flying.
			if (KeyHeld(input, KEY_SPACE))
				accel.y = 1.0f;

			if (KeyHeld(input, KEY_SHIFT)) 
				accel.y = -1.0f;
		} break;

		case MOVE_SWIMMING:
		{
			player->speed = player->walkSpeed;
			player->friction = -10.0f;
			gravity = 0.0f;

			bool lower, upper;
			GetPlayerInsideFluid(world, lower, upper);

			if (!upper && !lower)
				player->moveState = MOVE_NORMAL;
			else
			{
				player->surface = SURFACE_WATER;

				if (KeyHeld(input, KEY_SPACE))
					accel.y = 0.6f;

				if (KeyHeld(input, KEY_SHIFT)) 
					accel.y = -0.6f;
			}

		} break;
	}

	if (accel != vec3(0.0f))
		accel = normalize(accel);
}

static void Simulate(GameState* state, World* world, Player* player, float deltaTime)
{
	if (player->suspended) return;

	TIMED_FUNCTION;

	if (player->health <= 0)
	{
		KillPlayer(state, player);
		return;
	}

	Renderer& rend = state->renderer;
	Input& input = state->input;
	Camera* cam = state->camera;

	for (OverTimeDamage& ot : player->overTimeDamage)
		ot.active = false;

	vec3 accel = vec3(0.0f);

	if (KeyHeld(input, KEY_UP)) accel += cam->forward;
	if (KeyHeld(input, KEY_DOWN)) accel += -cam->forward;
	if (KeyHeld(input, KEY_LEFT)) accel += -cam->right;
	if (KeyHeld(input, KEY_RIGHT)) accel += cam->right;

	if (KeyPressed(input, KEY_TAB))
	{
		if (player->moveState == MOVE_FLYING)
			player->moveState = MOVE_NORMAL;
		else player->moveState = MOVE_FLYING;
	}

	float gravity;
	
	SetMoveParams(world, player, input, accel, gravity);
	MovePlayer(state, world, accel, deltaTime, gravity);

	// Delete impassible blocks the player has entered.
	vec3& p = player->pos;
	ivec3 lowerMid = BlockPos(vec3(p.x, p.y - 0.45f, p.z));
	ivec3 upperMid = ivec3(lowerMid.x, lowerMid.y + 1, lowerMid.z);

	if (!IsPassable(world, GetBlock(world, lowerMid)))
		SetBlock(world, lowerMid, BLOCK_AIR);

	if (!IsPassable(world, GetBlock(world, upperMid)))
		SetBlock(world, upperMid, BLOCK_AIR);

	ApplyOverTimeDamage(state, player, deltaTime);

	// Clamp the player within the loaded world region.
	float min = 0.0f, max = (float)(world->size * CHUNK_SIZE_H - 1);
	player->pos.x = Clamp(player->pos.x, min, max);
	player->pos.z = Clamp(player->pos.z, min, max);

	MoveCamera(cam, player->pos);
	UpdateCameraVectors(cam);

	// Tint the screen if the camera is in water. Subtract 0.1 to account
	// for the near clip plane of the camera.
	vec3 eyePos = vec3(cam->pos.x, cam->pos.y - 0.1f, cam->pos.z);
	Block eyeBlock = GetBlock(world, BlockPos(eyePos));

	if (IsFluid(world, eyeBlock))
	{
		Color tint = GetScreenTint(world, eyeBlock);

		if (rend.fadeColor.a == 0.0f)
		{
			SetScreenFade(rend, tint, FADE_PRIORITY_FLUID);
			rend.disableFluidCull = true;
		}
	}
	else
	{
		if (rend.fadePriority <= FADE_PRIORITY_FLUID)
		{
			SetScreenFade(rend, CLEAR_COLOR, FADE_PRIORITY_NONE);
			rend.disableFluidCull = false;
		}
	}

	HandleEditInput(state, input, world, deltaTime);
}

// Creates and spawns the player. The player is spawned within the center local space chunk.
static Player* NewPlayer(GameState* state)
{
	Player* player = new Player();
	
	player->velocity = vec3(0.0f);
	player->colFlags = HIT_NONE;
	player->spawned = false;
	player->moveState = MOVE_NORMAL;
	player->maxHealth = 10;
	player->health = player->maxHealth;
	player->damageFade = Color(1.0f, 0.0f, 0.0f, 0.5f);
	player->walkSpeed = 50.0f;
	player->flySpeed = 200.0f;
	player->jumpVelocity = 10.0f;

	RegisterCommand(state, "speed", PlayerSpeedCommand, player);
	RegisterCommand(state, "flyspeed", PlayerFlySpeedCommand, player);
	RegisterCommand(state, "health", PlayerHealthCommand, player);
	RegisterCommand(state, "kill", PlayerKillCommand, player);
	RegisterCommand(state, "jump", PlayerJumpCommand, player);

	return player;
}

static vec2 TryFindLand(World* world)
{
	ChunkGroup* group = GetGroup(world, world->loadRange, world->loadRange);

	vec2 center = vec2(CHUNK_SIZE_H * 0.5f);
	float closestDist = FLT_MAX;
	vec2 closestP = center;

	for (int z = 0; z < CHUNK_SIZE_H; z++)
	{
		for (int x = 0; x < CHUNK_SIZE_H; x++)
		{
			for (int y = WORLD_BLOCK_HEIGHT - 1; y >= 0; y--)
			{
				Block block = GetBlock(group, x, y, z);
				
				if (!IsPassable(world, block))
				{
					float dist = distance2(center, vec2(x, z));

					if (dist < closestDist)
					{
						closestDist = dist;
						closestP = vec2(x, z);
					}

					break;
				}
				else
				{
					if (block != BLOCK_AIR)
						break;
				}
			}
		}
	}

	return closestP;
}

static void SpawnPlayer(GameState* state, World* world, Player* player, Rectf spawnBounds)
{
	vec2 p = TryFindLand(world); 

	vec3 spawn = spawnBounds.min;
	spawn.x += p.x;
	spawn.z += p.y;

	Ray ray = { vec3(spawn.x, WORLD_BLOCK_HEIGHT, spawn.z), vec3(0.0, -1.0f, 0.0f) };

	vec3 loc;
	
	if (VoxelRaycast(world, ray, WORLD_BLOCK_HEIGHT, &loc))
		spawn.y = loc.y + 2.0f;
	else spawn.y = 258.0f;
	
	player->pos = spawn;

	MoveCamera(state->camera, player->pos);
	UpdateCameraVectors(state->camera);

	SetHomePos(world, player);
	player->spawned = true;
}
