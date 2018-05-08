// Voxel Engine
// Jason Bricco

inline ivec3 ToLocalPos(int wX, int wY, int wZ)
{
	return ivec3(wX & CHUNK_SIZE - 1, wY, wZ & CHUNK_SIZE - 1);
}

inline ivec3 ToLocalPos(ivec3 wPos)
{
	return ToLocalPos(wPos.x, wPos.y, wPos.z);
}

inline ivec3 ToChunkPos(int wX, int wZ)
{
	return ivec3(wX >> CHUNK_SIZE_BITS, 0, wZ >> CHUNK_SIZE_BITS);
}

inline ivec3 ToChunkPos(ivec3 wPos)
{
	return ToChunkPos(wPos.x, wPos.z);
}

inline ivec3 ToChunkPos(vec3 wPos)
{
	return ToChunkPos((int)wPos.x, (int)wPos.z);
}

inline int ChunkIndex(World* world, int cX, int cZ)
{
	return cZ * world->width + cX;
}

inline Chunk* GetChunk(World* world, int cX, int cZ)
{
	return world->chunks[ChunkIndex(world, cX, cZ)];
}

inline Chunk* GetChunk(World* world, ivec3 cPos)
{
	return GetChunk(world, cPos.x, cPos.z);
}

inline void MoveChunk(World* world, Chunk* chunk, int toX, int toZ)
{
	world->chunks[ChunkIndex(world, toX, toZ)] = chunk;
}

inline uint32_t ChunkHashBucket(int32_t x, int32_t z)
{
	uint32_t hashValue = (31 + x) * 23 + z;
	return hashValue & (CHUNK_HASH_SIZE - 1);
}

inline bool IsCorrect(Chunk* chunk, int32_t wX, int32_t wZ)
{
	if (chunk == NULL) return false;
	return chunk->wX == wX && chunk->wZ == wZ;
}

static Chunk* ChunkFromHash(World* world, uint32_t bucket, int32_t wX, int32_t wZ)
{
    Chunk* chunk = world->chunkHash[bucket];

    if (chunk == NULL) return NULL;

    if (IsCorrect(chunk, wX, wZ))
    {
    	world->chunkHash[bucket] = NULL;
    	return chunk;
    }
 
 	Chunk* prev = chunk;
 	chunk = chunk->nextInHash;

    while (chunk != NULL)
    {
        if (IsCorrect(chunk, wX, wZ))
        {
        	prev->nextInHash = chunk->nextInHash;
            return chunk;
        }
 
 		prev = chunk;
        chunk = chunk->nextInHash;
    }
 
    return NULL;
}

inline Chunk* ChunkFromHash(World* world, int32_t wX, int32_t wZ)
{
	 return ChunkFromHash(world, ChunkHashBucket(wX, wZ), wX, wZ);
}

inline void AddChunkToHash(World* world, Chunk* chunk)
{
	if (chunk == NULL) return;

	uint32_t bucket = ChunkHashBucket(chunk->wX, chunk->wZ);
	Chunk* existing = ChunkFromHash(world, bucket, chunk->wX, chunk->wZ);

	if (existing == NULL)
		world->chunkHash[bucket] = chunk;
	else 
	{
		while (true)
		{
			if (existing->nextInHash == NULL)
			{
				existing->nextInHash = chunk;
				return;
			}

			existing = existing->nextInHash;
		}
	}
}

inline void SetBlock(Chunk* chunk, int lX, int lY, int lZ, int block)
{
	chunk->blocks[lX + CHUNK_SIZE * (lY + WORLD_HEIGHT * lZ)] = block;
}

inline void SetBlock(Chunk* chunk, ivec3 lPos, int block)
{
	SetBlock(chunk, lPos.x, lPos.y, lPos.z, block);
}

inline void SetBlock(World* world, ivec3 wPos, int block, bool update)
{
	if (wPos.y < 0 || wPos.y >= WORLD_HEIGHT) return;

	ivec3 cPos = ToChunkPos(wPos);
	Chunk* chunk = GetChunk(world, cPos);
	Assert(chunk != NULL);

	ivec3 local = ToLocalPos(wPos);
	SetBlock(chunk, local, block);

	if (update) UpdateChunk(world, chunk, local);
}

inline void SetBlock(World* world, int wX, int wY, int wZ, int block, bool update)
{
	SetBlock(world, ivec3(wX, wY, wZ), block, update);
}

inline int GetBlock(Chunk* chunk, int lX, int lY, int lZ)
{
	return chunk->blocks[lX + CHUNK_SIZE * (lY + WORLD_HEIGHT * lZ)];
}

inline int GetBlock(Chunk* chunk, ivec3 lPos)
{
	return GetBlock(chunk, lPos.x, lPos.y, lPos.z);
}

static int GetBlock(World* world, int wX, int wY, int wZ)
{
	if (wY < 0 || wY >= WORLD_HEIGHT) return 0;

	ivec3 cPos = ToChunkPos(wX, wZ);
	Chunk* chunk = GetChunk(world, cPos.x, cPos.z);
	Assert(chunk != NULL);

	ivec3 lPos = ToLocalPos(wX, wY, wZ);
	return GetBlock(chunk, lPos);
}

inline int GetBlock(World* world, ivec3 pos)
{
	return GetBlock(world, pos.x, pos.y, pos.z);
}

static void GenerateChunkTerrain(Noise* noise, Chunk* chunk, int startX, int startZ)
{
	BEGIN_TIMED_BLOCK(GEN_TERRAIN);

	// Base terrain.
	noise->SetFractalOctaves(2);
	noise->SetFractalGain(0.32);
	float* terrain = noise->GetSimplexFractalSet(startX, 0, startZ, CHUNK_SIZE, 1, CHUNK_SIZE, 0.6f); 

	// Mountain mask.
	float* mountainMask = noise->GetSimplexSet(startX, 0, startZ, CHUNK_SIZE, 1, CHUNK_SIZE, 1.0f);

	// Mountains.
	noise->SetFractalType(Noise::RigidMulti);
	noise->SetFractalOctaves(6);
	noise->SetFractalGain(0.5f);
	float* mountain = noise->GetSimplexFractalSet(startX * 4, 0, startZ * 4, CHUNK_SIZE, 1, CHUNK_SIZE, 1.0f);

	int i = 0;

	for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
        	float val = terrain[i];
        	float height = (val * 15) + 20.0f;

        	float mask = (mountainMask[i] + 2.0f) / 2.0f;

        	// Create mountains.
        	if (mask > 0.6f)
        	{
        		float rawHeight = mountain[i] * 50.0f;
        		float actualHeight = rawHeight * Min(1.0f, (mask - 0.6f) / 0.2f);
        		height += actualHeight;
        	}
        	
        	int terrainHeight = Clamp((int)height, 1, WORLD_HEIGHT - 1);

   			i++;

        	for (int y = 0; y <= terrainHeight; y++)
        	{
        		if (y < terrainHeight - 5)
        			SetBlock(chunk, x, y, z, BLOCK_STONE);
        		else if (y == terrainHeight)
        			SetBlock(chunk, x, y, z, BLOCK_GRASS);
        		else SetBlock(chunk, x, y, z, BLOCK_DIRT);
        	}
        }
    }

    _aligned_free(terrain);

    END_TIMED_BLOCK(GEN_TERRAIN);

	#if 0
	int index2 = 0, index3 = -1;

	for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
        	int terrainHeight = Clamp((int)((terrain[index2] * 10) + 20.0f), 1, WORLD_HEIGHT - 1);
        	int islandHeight = Clamp((int)((island[index2++] * 50) + 20.0f), 1, WORLD_HEIGHT - 1);

        	int maxHeight = Max(terrainHeight, islandHeight);

        	for (int y = 0; y <= maxHeight; y++)
        	{
        		index3++;

        		if (y <= terrainHeight) 
				{
					float value = terrain3D[index3];
					
					if (IsInRange(value, -1.0f, 0.2f))
						SetBlock(chunk, x, y, z, y == maxHeight ? BLOCK_GRASS : BLOCK_DIRT);

					if (IsInRange(value, 0.2f, 1.0f)) 
						SetBlock(chunk, x, y, z, BLOCK_STONE);

					continue;
				}

				if (y <= islandHeight) 
				{
					float value = island3D[index3];
					
					if (IsInRange(value, 0.4f, 0.6f))
						SetBlock(chunk, x, y, z, y == maxHeight ? BLOCK_GRASS : BLOCK_DIRT);

					if (IsInRange(value, 0.6f, 1.0f))
						SetBlock(chunk, x, y, z, BLOCK_STONE);

					continue;
				}
        	}
        }
    }

	_aligned_free(terrain3D);
	_aligned_free(island);
	_aligned_free(island3D);
	#endif
}

static void FillChunk(Chunk* chunk, int block)
{
	for (int i = 0; i < CHUNK_SIZE_3; i++)
		chunk->blocks[i] = block;
}

inline void AddChunkToPool(World* world, Chunk* chunk)
{
	memset(chunk, 0, sizeof(Chunk));
	world->pool[world->poolSize++] = chunk;
}

inline Chunk* ChunkFromPool(World* world)
{
	Assert(world->poolSize > 0);
	Chunk* chunk = world->pool[world->poolSize - 1];
	world->poolSize--;
	return chunk;
}

static Chunk* CreateChunk(World* world, int cX, int cZ, int wX, int wZ)
{
	int index = ChunkIndex(world, cX, cZ);
	Chunk* chunk = world->chunks[index];

	if (chunk == NULL)
	{
		chunk = ChunkFromPool(world);
		chunk->cX = cX;
		chunk->cZ = cZ;
		chunk->wX = wX;
		chunk->wZ = wZ;
		GenerateChunkTerrain(world->noise, chunk, wX * CHUNK_SIZE, wZ * CHUNK_SIZE);
		world->chunks[index] = chunk;
	}

	return chunk;
}

static void DestroyChunk(World* world, Chunk* chunk)
{
	if (chunk->nextInHash != NULL)
		DestroyChunk(world, chunk->nextInHash);

	if (chunk->state == CHUNK_BUILT)
		DestroyMesh(chunk->mesh);

	AddChunkToPool(world, chunk);
}

inline void BuildBlock(World* world, Mesh* mesh, float x, float y, float z, int wX, int wY, int wZ, int block)
{
	float* textures = world->blockData[block].textures;

	// Top face.
	if (GetBlock(world, wX, wY + 1, wZ) == 0)
	{
		float tex = textures[FACE_TOP];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Bottom face.
	if (GetBlock(world, wX, wY - 1, wZ) == 0)
	{
		float tex = textures[FACE_BOTTOM];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Front face.
	if (GetBlock(world, wX, wY, wZ + 1) == 0)
	{
		float tex = textures[FACE_FRONT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f); 
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Back face.
	if (GetBlock(world, wX, wY, wZ - 1) == 0)
	{
		float tex = textures[FACE_BACK];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Right face.
	if (GetBlock(world, wX + 1, wY, wZ) == 0)
	{
		float tex = textures[FACE_RIGHT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Left face.
	if (GetBlock(world, wX - 1, wY, wZ) == 0)
	{
		float tex = textures[FACE_LEFT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}

static void BuildChunk(World* world, Chunk* chunk)
{
	BEGIN_TIMED_BLOCK(BUILD_CHUNK);

	chunk->mesh = CreateMesh();

	for (int y = 0; y < WORLD_HEIGHT; y++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				int block = GetBlock(chunk, x, y, z);

				if (block != BLOCK_AIR)
				{
					int wX = chunk->cX * CHUNK_SIZE;
					int wZ = chunk->cZ * CHUNK_SIZE;
					BuildBlock(world, chunk->mesh, (float)x, (float)y, (float)z, wX + x, y, wZ + z, block);
				}
			}
		}
	}
	
	FillMeshData(chunk->mesh);
	chunk->state = CHUNK_BUILT;

	END_TIMED_BLOCK(BUILD_CHUNK);
}

inline void UpdateChunkDirect(World* world, Chunk* chunk)
{
	if (chunk->state == CHUNK_BUILT)
	{
		DestroyMesh(chunk->mesh);
		BuildChunk(world, chunk);
	}
}

inline void UpdateChunk(World* world, Chunk* chunk, ivec3 lPos)
{
	UpdateChunkDirect(world, chunk);

	int32_t cX = chunk->cX, cZ = chunk->cZ;

	if (lPos.x == 0) UpdateChunkDirect(world, GetChunk(world, cX - 1, cZ));
	else if (lPos.x == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cX + 1, cZ));
	
	if (lPos.z == 0) UpdateChunkDirect(world, GetChunk(world, cX, cZ - 1));
	else if (lPos.z == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cX, cZ + 1));
}

static void ShiftWorld(World* world)
{
	int edge = world->width - 1;

	for (int i = 0; i < world->totalChunks; i++)
	{
		AddChunkToHash(world, world->chunks[i]);
		world->chunks[i] = NULL;
	}

	for (int z = 0; z <= edge; z++)
	{
		for (int x = 0; x <= edge; x++)
		{
			int wX = world->refX + x;
			int wZ = world->refZ + z;

			Chunk* chunk = ChunkFromHash(world, wX, wZ);

			if (chunk == NULL)
				CreateChunk(world, x, z, wX, wZ);
			else 
			{
				chunk->cX = x;
				chunk->cZ = z;
				world->chunks[ChunkIndex(world, x, z)] = chunk;
			}
		}
	}

	for (int c = 0; c < CHUNK_HASH_SIZE; c++)
	{
		Chunk* chunk = world->chunkHash[c];

		if (chunk == NULL)
			continue;

		DestroyChunk(world, chunk);
		world->chunkHash[c] = NULL;
	}

	for (int z = 1; z < edge; z++)
	{
		for (int x = 1; x < edge; x++)
		{
			Chunk* chunk = GetChunk(world, x, z);

			if (chunk->state == CHUNK_GENERATED)
				BuildChunk(world, chunk);
		}
	}
}

static World* NewWorld(int loadRange)
{
	World* world = Calloc(World);

	// Load range worth of chunks on each side, the middle chunk, and two boundary
	// chunks form the total width.
	world->width = (loadRange * 2) + 3;

	world->spawnX = 0;
	world->spawnZ = 0;

	world->totalChunks = Square(world->width);
	world->chunks = (Chunk**)calloc(1, world->totalChunks * sizeof(Chunk*));
	world->refX = world->spawnX - loadRange - 1;
	world->refZ = world->spawnZ - loadRange - 1;

	// Allocate extra chunks for the pool for world shifting. We create new chunks
	// before we destroy the old ones.
	int targetPoolSize = world->totalChunks + world->width;
	world->pool = (Chunk**)calloc(1, targetPoolSize * sizeof(Chunk*));
	world->poolSize = 0;

	for (int i = 0; i < targetPoolSize; i++)
		AddChunkToPool(world, Malloc(Chunk));

	world->pMin = (float)((loadRange + 1) * CHUNK_SIZE);
	world->pMax = world->pMin + CHUNK_SIZE;

	CreateBlockData(world->blockData);

	world->noise = FastNoiseSIMD::NewFastNoiseSIMD();
	world->noise->SetSeed(rand());

	ShiftWorld(world);

	return world;
}
