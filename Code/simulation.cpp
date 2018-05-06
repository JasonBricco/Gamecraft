// Voxel Engine
// Jason Bricco

static Player* NewPlayer(float pMin, float pMax)
{
	Player* player = Malloc(Player);
	player->camera = NewCamera();

	float spawn = pMin + (CHUNK_SIZE / 2.0f);
	player->pos = NewV3(spawn, 80.0f, spawn);
	player->collider = Capsule(0.3f, 1.2f);
	player->velocity = NewV3(0.0f);
	player->speed = 50.0f;
	player->friction = -8.0f;
	player->colFlags = HIT_NONE;
	player->flying = false;
	player->speedMode = false;

	return player;
}

AABB::AABB(Vec3 p, Vec3 minP, Vec3 maxP)
{
	pos = p;
	min = minP;
	max = maxP;
}

Capsule::Capsule(float radius, float h)
{
	pos = NewV3(0.0f);
	r = radius;
	yBase = 0.0f;
	yTop = h;
}

// Returns the farthest point along an AABB in the given direction in world space.
inline Vec3 AABB::Support(Vec3 dir)
{
	Vec3 result;
	result.x = dir.x > 0.0f ? max.x : min.x;
	result.y = dir.y > 0.0f ? max.y : min.y;
	result.z = dir.z > 0.0f ? max.z : min.z;

	return pos + result;
}

// Returns the farthest point along a capsule in the given direction in world space.
inline Vec3 Capsule::Support(Vec3 dir)
{
	Vec3 result = Normalize(dir) * r;
	result.y += dir.y > 0.0f ? yTop : yBase;

	return pos + result;
}

static float BlockRayIntersection(Vec3 blockPos, Ray ray)
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

static bool VoxelRaycast(World* world, Ray ray, float dist, Vec3* result)
{
	Vec3i start = BlockPos(ray.origin);
	Vec3i end = BlockPos(ray.origin + ray.dir * dist);

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
				int block = GetBlock(world, x, y, z);

				if (block == 0) continue;

				float newDist = BlockRayIntersection(NewV3((float)x, (float)y, (float)z), ray);
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

static HitInfo GetVoxelHit(Renderer* rend, World* world)
{
	HitInfo info = {};
	Ray ray = ScreenCenterToRay(rend);

	Vec3 point;
	
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

static void UpdateSimplex3(Vec3& a, Vec3& b, Vec3& c, Vec3& d, int& dim, Vec3& search)
{
	// Triangle normal.
	Vec3 norm = Cross(b - a, c - a);

	// Direction to the origin.
	Vec3 ao = -a;

	// Determine which feature of the triangle is closest to the origin and make it
	// the new simplex - any of its edges, or in front of or behind it. 
	dim = 2;

	// Origin is closest to edge 'ab'.
	if (Dot(Cross(b - a, norm), ao) > 0)
	{
		c = a;
		search = Cross(Cross(b - a, ao), b - a);
		return;
	}

	// Origin is closest to edge 'ac'.
	if (Dot(Cross(norm, c - a), ao) > 0)
	{
		b = a;
		search = Cross(Cross(c - a, ao), c - a);
		return;
	}

	dim = 3;

	// Origin is above the triangle.
	if (Dot(norm, ao) > 0)
	{
		d = c;
		c = b;
		b = a;
		search = norm;
		return;
	}

	// Origin is below the triangle.
	d = b;
	b = a;
	search = -norm;
}

static bool UpdateSimplex4(Vec3& a, Vec3& b, Vec3& c, Vec3& d, int& dim, Vec3& search)
{
	// Normals of the three non-base tetrahedron faces.
	Vec3 abc = Cross(b - a, c - a);
	Vec3 acd = Cross(c - a, d - a);
	Vec3 adb = Cross(d - a, b - a);

	Vec3 ao = -a;
	dim = 3;

	// Origin is in front of 'abc'.
	if (Dot(abc, ao) > 0)
	{
		d = c;
		c = b;
		b = a;
		search = abc;
		return false;
	}

	// Origin is in front of 'acd'.
	if (Dot(acd, ao) > 0)
	{
		b = a;
		search = acd;
		return false;
	}

	// Origin is in front of 'adb'.
	if (Dot(adb, ao) > 0)
	{
		c = d;
		d = b;
		b = a;
		search = adb;
		return false;
	}

	return true;
}

static CollisionInfo EPA(Vec3 a, Vec3 b, Vec3 c, Vec3 d, Collider* colA, Collider* colB)
{
	// Each triangle face has three vertices and a normal.
	Vec3 faces[64][4];

	// Begin the array with the final simplex from GJK.
	faces[0][0] = a;
	faces[0][1] = b;
	faces[0][2] = c;
	faces[0][3] = Normalize(Cross(b - a, c - a)); 

	faces[1][0] = a;
	faces[1][1] = c;
	faces[1][2] = d;
	faces[1][3] = Normalize(Cross(c - a, d - a));

	faces[2][0] = a;
	faces[2][1] = d;
	faces[2][2] = b;
	faces[2][3] = Normalize(Cross(d - a, b - a));

	faces[3][0] = b;
	faces[3][1] = d;
	faces[3][2] = c;
	faces[3][3] = Normalize(Cross(d - b, c - b));

	int faceCount = 4;
	int closest;

	for (int iter = 0; iter < 32; iter++)
	{
		// Find the face that's closest to the origin.
		float minDist = Dot(faces[0][0], faces[0][3]);
		closest = 0;

		for (int i = 1; i < faceCount; i++)
		{
			float dist = Dot(faces[i][0], faces[i][3]);

			if (dist < minDist)
			{
				minDist = dist;
				closest = i;
			}
		}

		// Normal of the face closest to the origin.
		Vec3 search = faces[closest][3];

		Vec3 p = colB->Support(search) - colA->Support(-search);

		// Dot product between the vertex and normal gives the resolution of the collision along the normal. 
		if (Dot(p, search) - minDist < EPA_TOLERANCE)
		{
			Vec3 mtv = faces[closest][3] * Dot(p, search); 
			return { mtv, faces[closest][3] };
		}

		// Tracks edges that must be fixed after removing faces.
		Vec3 looseEdges[32][2];
		int looseCount = 0;

		// Find all triangles facing point p.
		for (int i = 0; i < faceCount; i++)
		{
			// If triangle i faces p, remove it.
			if (Dot(faces[i][3], p - faces[i][0]) > 0)
			{
				// Add removed triangle's edges to loose edge list but remove it if it's already there.
				for (int j = 0; j < 3; j++)
				{
					Vec3 currentEdge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found = false;

					// Checks to see if the current edge is already in the list.
					for (int k = 0; k < looseCount; k++)
					{
						if (looseEdges[k][1] == currentEdge[0] && looseEdges[k][0] == currentEdge[1])
						{
							// Edge is already in the list, remove it. 
							looseEdges[k][0] = looseEdges[looseCount - 1][0];
							looseEdges[k][1] = looseEdges[looseCount - 1][1];
							looseCount--;
							found = true;

							// Exit loop as the edge can only be shared once.
							k = looseCount; 
						}
					}

					if (!found)
					{
						// Add current edge to the list.
						if (looseCount > 32) break;

						looseEdges[looseCount][0] = currentEdge[0];
						looseEdges[looseCount][1] = currentEdge[1];
						looseCount++;
					}
				}

				faces[i][0] = faces[faceCount - 1][0];
				faces[i][1] = faces[faceCount - 1][1];
				faces[i][2] = faces[faceCount - 1][2];
				faces[i][3] = faces[faceCount - 1][3];
				faceCount--;
				i--;
			}
		}

		// Reconstruct the polytope with point p added.
		for (int i = 0; i < looseCount; i++)
		{
			if (faceCount > 64) break;

			faces[faceCount][0] = looseEdges[i][0];
			faces[faceCount][1] = looseEdges[i][1];
			faces[faceCount][2] = p;
			faces[faceCount][3] = Normalize(Cross(looseEdges[i][0] - looseEdges[i][1], looseEdges[i][0] - p));

			// Check for the wrong normal to maintain counterclockwise winding.
			float bias = 0.00001f;

			if (Dot(faces[faceCount][0], faces[faceCount][3]) + bias < 0)
			{
				Vec3 temp = faces[faceCount][0];
				faces[faceCount][0] = faces[faceCount][1];
				faces[faceCount][1] = temp;
				faces[faceCount][3] = -faces[faceCount][3];
			}

			faceCount++;
		}
	}

	Vec3 mtv = faces[closest][3] * Dot(faces[closest][0], faces[closest][3]);
	return { mtv, faces[closest][3] };
}

static bool Intersect(Collider* colA, Collider* colB, CollisionInfo* info)
{
	Vec3 a, b, c, d;
	Vec3 search = colA->pos - colB->pos;

	// Initial simplex point.
	c = colB->Support(search) - colA->Support(-search);

	// Search in the direction of the origin.
	search = -c;

	// Second point to form a line segment of the simplex.
	b = colB->Support(search) - colA->Support(-search);

	// We haven't reached the origin, so we can't enclose it.
	if (Dot(b, search) < 0) return false;

	// Search perpendicular to the line segment, toward the origin.
	search = Cross(Cross(c - b, -b), c - b);

	// Origin is on the line segment we created.
	if (search == NewV3(0.0f))
	{
		// Set search to an arbitrary normal vector. In this case, use the x-axis.
		search = Cross(c - b, NewV3(1.0f, 0.0f, 0.0f));

		// If we're still on the line segment, normal with the z-axis.
		if (search == NewV3(0.0f))
			search = Cross(c - b, NewV3(0.0f, 0.0f, -1.0f)); 
	}

	// Number of simplex dimensions.
	int dim = 2;

	for (int iter = 0; iter < 32; iter++)
	{
		a = colB->Support(search) - colA->Support(-search);

		// We cannot enclose the origin as we haven't reached it.
		if (Dot(a, search) < 0) return false;

		dim++;

		if (dim == 3) UpdateSimplex3(a, b, c, d, dim, search);
		else
		{
			if (UpdateSimplex4(a, b, c, d, dim, search))
			{
				if (info != NULL)
					*info = EPA(a, b, c, d, colA, colB);
				
				return true;
			}
		}
	}

	return false;
}

inline void CameraFollow(Player* player)
{	
	Vec3 pos = player->pos;
	player->camera->pos = NewV3(pos.x, pos.y + 1.15f, pos.z);
	UpdateCameraVectors(player->camera);
}

static void Move(World* world, Player* player, Vec3 accel, float deltaTime)
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
	Vec3 delta = accel * 0.5f * Square(deltaTime) + player->velocity * deltaTime;
	player->velocity = accel * deltaTime + player->velocity;

	// Skip collision detection if flying.
	if (player->flying)
		player->pos = player->pos + delta;
	else
	{
		BEGIN_TIMED_BLOCK(PLAYER_COLLISION);

		player->colFlags = HIT_NONE;
		Capsule col = player->collider;

		// Player size in blocks.
		int blockR = CeilToInt(col.r);
		int blockH = CeilToInt(col.yTop - col.yBase);
		Vec3i bSize = NewV3i(blockR, blockH, blockR);

		float deltaLen = Length(delta);

		// If our move is too big, try to prevent skipping through terrain.
		if (deltaLen > 1.5f)
		{
			Ray ray = { player->pos, Normalize(delta) };

			// Move the player to the terrain if the ray hits it. If not, move the full distance.
			Vec3 result;
			if (VoxelRaycast(world, ray, deltaLen, &result))
				player->pos = result;
			else player->pos = player->pos + delta;
		}
		else player->pos = player->pos + delta;

		Vec3i newBlock = BlockPos(player->pos);

		// Compute the range of blocks we could touch with our movement. We'll test for collisions
		// with the blocks in this range.
		int minX = newBlock.x - bSize.x;
		int minY = newBlock.y - bSize.y;
		int minZ = newBlock.z - bSize.z;

		int maxX = newBlock.x + bSize.x;
		int maxY = newBlock.y + bSize.y;
		int maxZ = newBlock.z + bSize.z;

		CollisionInfo info;

		for (int y = minY; y <= maxY; y++)
		{
			for (int z = minZ; z <= maxZ; z++)
			{
				for (int x = minX; x <= maxX; x++)
				{
					int block = GetBlock(world, x, y, z);

					if (block != 0)
					{
						col.pos = player->pos;
						AABB bb = AABB(NewV3(x - 0.5f, y - 0.5f, z - 0.5f), NewV3(0.0f), NewV3(1.0f));
						
						if (Intersect(&col, &bb, &info))
						{
							player->pos = player->pos + info.mtv;

							if (info.normal.y > 0.25f)
							{
								player->colFlags |= HIT_DOWN;
								player->velocity.y = 0.0f;
							}
						}
					}
				}
			}
		}

		END_TIMED_BLOCK(PLAYER_COLLISION);
	}

	CameraFollow(player);
}

static void Simulate(Renderer* rend, World* world, Player* player, float deltaTime)
{
	Camera* cam = player->camera;

	Vec3 accel = NewV3(0.0f);

	if (KeyHeld(KEY_UP)) accel = MoveDirXZ(cam->forward);
	if (KeyHeld(KEY_DOWN)) accel = MoveDirXZ(-cam->forward);
	if (KeyHeld(KEY_LEFT)) accel = MoveDirXZ(-cam->right);
	if (KeyHeld(KEY_RIGHT)) accel = MoveDirXZ(cam->right);

	if (KeyPressed(KEY_TAB))
		player->flying = !player->flying;

	if (KeyPressed(KEY_P))
		player->speedMode = !player->speedMode;

	if (player->flying)
	{
		player->speed = player->speedMode ? 5000.0f : 200.0f;

		if (KeyHeld(KEY_SPACE))
			accel.y = 1.0f;

		if (KeyHeld(KEY_SHIFT)) accel.y = -1.0f;
	}
	else 
	{
		player->speed = 50.0f;

		if ((player->colFlags & HIT_DOWN) && KeyHeld(KEY_SPACE))
			player->velocity.y = 15.0f;
	}

	Move(world, player, accel, deltaTime);

	int op = MousePressed(0) ? 0 : MousePressed(1) ? 1 : -1;

	if (op >= 0)
	{
		HitInfo info = GetVoxelHit(rend, world);

		if (info.hit)
		{
			Vec3i setPos;

			if (op == 0)
			{
				setPos = info.adjPos;
				SetBlock(world, setPos, 1, true);
			}
			else
			{
				setPos = info.hitPos;
				SetBlock(world, setPos, 0, true);
			}
		}
	}
}
