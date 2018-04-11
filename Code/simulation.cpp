// Voxel Engine
// Jason Bricco

static Player* NewPlayer(vec3 pos)
{
	Player* player = Malloc(Player);
	player->camera = NewCamera(pos);
	player->pos = pos;
	player->col = { vec3(0.0f), vec3(0.5f, 0.9f, 0.5f) };
	player->speed = 200.0f;
	player->friction = -8.0f;
	player->collisionFlags = HIT_NONE;

	return player;
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

	// Find all blocks between where the entity was and where the entity will be. Determine if 
	// any of those blocks are obstacles to collide with. If so, test against its six walls by 
	// solving for the time the entity will collide with them if they were infinite in length. 
	// When we detect the collision, we want to check the other axes to see the range. That way we won't 
	// handle collisions outside the range of a tile. If we didn't do that, we would never get past the 
	// wall (infinite assumption). The equation for solving for a collision is:

	// - f(t) = p0 + td = w (using linear interpolation). For an individual coordinate, say x, we would do:

	// p0x + tdx = wx
	// tdx = wx - p0x
	// t = (wx - p0x) / dx, where dx cannot be 0.

	// Where p0 is our starting location, t is the interpolation amount, and d is the movement vector.
	// We can analyze this using any of the individual vector components.

	// Position we will try to move to.
	vec3 target = player->pos + delta;

	Collider col = player->col;

	// Player size in blocks.
	ivec3 bSize(CeilToInt(col.size.x), CeilToInt(col.size.y), CeilToInt(col.size.z));

	player->collisionFlags = HIT_NONE;

	// Try to resolve collisions for four iterations.
	for (int iter = 0; iter < 4; iter++)
	{
		// Used to determine the closest collision we've found. We set it to 1 initially 
		// in order to consider it the full movement (plugged in for t in the above equation.) The goal is 
		// to find the collision closest to the entity.
		float tMin = 1.0f;

		if (Approx(length(delta), 0.0f)) break;

		vec3 normal = vec3(0.0f);
		ivec3 bPos = BlockPos(player->pos + col.offset);

		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				for (int x = -1; x <= 1; x++)
				{
					ivec3 targetBlock(bPos.x + x, bPos.y + y, bPos.z + z);
					int block = GetBlock(world, targetBlock);

					if (block != 0)
					{
						// Expanded diameter (minkowski sum).
						float diaX = 1.0f + col.size.x;
						float diaY = 1.0f + col.size.y;
						float diaZ = 1.0f + col.size.z;

						vec3 diff = (player->pos + col.offset) - GetVec3(targetBlock); 

						// Locations of the sides of the object we are colliding with relative to its center. 
						vec3 min = vec3(-diaX, -diaY, -diaZ) * 0.5f;
						vec3 max = vec3(diaX, diaY, diaZ) * 0.5f;

						Wall walls[] = 
						{
							// Right side.
							{ max.x, delta.x, delta.y, delta.z, diff.x, diff.y, diff.z, min.y, max.y, min.z, max.z,
								vec3(1.0f, 0.0f, 0.0f), HIT_OTHER },

							// Left side.
							{ min.x, delta.x, delta.y, delta.z, diff.x, diff.y, diff.z, min.y, max.y, min.z, max.z,
								vec3(-1.0f, 0.0f, 0.0f), HIT_OTHER },

							// Front side.
							{ max.z, delta.z, delta.y, delta.x, diff.z, diff.y, diff.x, min.y, max.y, min.x, max.x,
								vec3(0.0f, 0.0f, 1.0f), HIT_OTHER },

							// Back side.
							{ min.z, delta.z, delta.y, delta.x, diff.z, diff.y, diff.x, min.y, max.y, min.x, max.x,
								vec3(0.0f, 0.0f, -1.0f), HIT_OTHER },

							// Bottom side.
							{ min.y, delta.y, delta.x, delta.z, diff.y, diff.x, diff.z, min.x, max.x, min.z, max.z,
								vec3(0.0f, -1.0f, 0.0f), HIT_UP },

							// Top side.
							{ max.y, delta.y, delta.x, delta.z, diff.y, diff.x, diff.z, min.x, max.x, min.z, max.z,
								vec3(0.0f, 1.0f, 0.0f), HIT_DOWN }
						};

						bool hit = false;

						// Test for collisions against all six block sides.
						for (int i = 0; i < 6; i++)
						{
							Wall* wall = walls + i;

							if (wall->deltaA != 0.0f)
							{
								// tResult is the percentage of the movement vector we must travel along 
								// to collide with this wall. If it's larger than 1, then we haven't collided 
								// yet. If it's negative, we would have to go backwards to collide. 
								// Computed using linear interpolation. 
								float tResult = (wall->pos - wall->relA) / wall->deltaA;

								// See the value of the movement vector's perpendicular coordinate (from the 
								// wall we're testing) at the point of collision to determine if it's inside 
								// the block we're testing. If not, 
								// we can ignore it. If it is, it's a valid collision.
								float perpB = wall->relB + (tResult * wall->deltaB);
								float perpC = wall->relC + (tResult * wall->deltaC);

								// If the percentage of our move that will result in collision (tResult) is 
								// smaller than the one we've set previously (tMin), then this is a closer 
								// collision. We'll use it. Don't handle the collision if tResult is negative, 
								// as we won't support backwards collisions.
								if (tResult >= 0.0f && tMin > tResult)
								{
									if (perpB >= wall->minB && perpB <= wall->maxB && 
										perpC >= wall->minC && perpC <= wall->maxC)
									{
										tMin = Max(0.0f, tResult - EPSILON);
										
										player->collisionFlags |= wall->flag;
										normal = wall->normal;
										hit = true;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// CONTINUE COLLISION DETECTION HERE

	/*
	player->pos += delta;
	player->camera->pos += delta;
	player->col = test;
	UpdateCameraVectors(player->camera);*/
}
