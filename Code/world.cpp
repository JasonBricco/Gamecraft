// Voxel Engine
// Jason Bricco

inline bool BlockInsideWorld(World* world, int wX, int wY, int wZ);
static void BuildChunk(World* world, Chunk* chunk);

static World* NewWorld(int width, int length)
{
	World* world = Calloc(World);
	world->loadRangeH = 8;

	ivec3 sizeInChunks = ivec3(Min(width, WORLD_MAX_H), 1, Min(length, WORLD_MAX_H));
	world->sizeInChunks = sizeInChunks;
	world->sizeInBlocks = ivec3(sizeInChunks.x * CHUNK_SIZE, WORLD_BLOCK_HEIGHT, sizeInChunks.z * CHUNK_SIZE);
	world->spawn = vec3((width * CHUNK_SIZE) / 2, 80.0f, (length * CHUNK_SIZE) / 2);

	world->chunks = (Chunk**)calloc(1, width * length * sizeof(Chunk*));

	int memXZ = Square(((world->loadRangeH * 2) + 3));
	world->loadedChunks = (Chunk**)calloc(1, memXZ * sizeof(Chunk*));

	world->noise = FastNoiseSIMD::NewFastNoiseSIMD();
	world->noise->SetSeed(rand());

	world->lastLoadPos = ivec3(-1);

	return world;
}

inline int ChunkIndex(World* world, int cX, int cZ)
{
	return cZ * world->sizeInChunks.x + cX;
}

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

inline bool ChunkInsideWorld(World* world, int cX, int cZ)
{
	return cX >= 0 && cX < world->sizeInChunks.x && cZ >= 0 && cZ < world->sizeInChunks.z;
}

inline bool ChunkInsideWorld(World* world, ivec3 cPos)
{
	return ChunkInsideWorld(world, cPos.x, cPos.z);
}

inline Chunk* GetChunk(World* world, int cX, int cZ)
{
	if (!ChunkInsideWorld(world, cX, cZ))
		return NULL;

	return world->chunks[ChunkIndex(world, cX, cZ)];
}

inline Chunk* GetChunk(World* world, ivec3 cPos)
{
	return GetChunk(world, cPos.x, cPos.z);
}

inline void UpdateChunkDirect(World* world, Chunk* chunk)
{
	DestroyMesh(&chunk->mesh);
	BuildChunk(world, chunk);
}

inline void UpdateChunk(World* world, Chunk* chunk, ivec3 lPos)
{
	UpdateChunkDirect(world, chunk);

	ivec3 cPos = chunk->cPos;

	if (lPos.x == 0) UpdateChunkDirect(world, GetChunk(world, cPos.x - 1, cPos.z));
	else if (lPos.x == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cPos.x + 1, cPos.z));
	
	if (lPos.z == 0) UpdateChunkDirect(world, GetChunk(world, cPos.x, cPos.z - 1));
	else if (lPos.z == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cPos.x, cPos.z + 1));
}

inline void SetBlock(Chunk* chunk, int lX, int lY, int lZ, int block)
{
	chunk->blocks[lX + CHUNK_SIZE * (lY + WORLD_BLOCK_HEIGHT * lZ)] = block;
}

inline void SetBlock(Chunk* chunk, ivec3 lPos, int block)
{
	SetBlock(chunk, lPos.x, lPos.y, lPos.z, block);
}

static void SetBlock(World* world, int wX, int wY, int wZ, int block, bool update)
{
	if (!BlockInsideWorld(world, wX, wY, wZ)) return;

	ivec3 cPos = ToChunkPos(wX, wY);
	Chunk* chunk = GetChunk(world, cPos);

	if (chunk == NULL) return;

	ivec3 lPos = ToLocalPos(wX, wY, wZ);
	SetBlock(chunk, lPos, block);

	if (update) UpdateChunk(world, chunk, lPos);
}

inline void SetBlock(World* world, ivec3 wPos, int block, bool update)
{
	SetBlock(world, wPos.x, wPos.y, wPos.z, block, update);
}

static void GenerateChunkTerrain(Noise* noise, Chunk* chunk)
{
	ivec3 wPos = chunk->wPos;
	float* noiseSet = noise->GetSimplexSet(wPos.x, wPos.y, wPos.z, CHUNK_SIZE, 1, CHUNK_SIZE);   

	int index = 0;

	for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
        	int height = (int)((20.0f + noiseSet[index++]) * 3.0f);
        	height = Clamp(height, 0, WORLD_BLOCK_HEIGHT - 1);

        	for (int y = 0; y <= height; y++)
        		SetBlock(chunk, x, y, z, 1);
        }
    }
}

static void FillChunk(Chunk* chunk, int block)
{
	for (int i = 0; i < CHUNK_SIZE_3; i++)
		chunk->blocks[i] = block;
}

inline void SetChunkLoaded(World* world, Chunk* chunk)
{
	world->loadedChunks[world->loadedCount++] = chunk;
}

inline void SetChunkUnloaded(World* world, Chunk* chunk, int index)
{
	world->loadedChunks[index] = world->loadedChunks[world->loadedCount - 1];
	world->loadedChunks[world->loadedCount - 1] = NULL;
	world->loadedCount--;
}

static Chunk* CreateChunk(World* world, int cX, int cZ)
{
	if (!ChunkInsideWorld(world, cX, cZ))
		return NULL;

	int index = ChunkIndex(world, cX, cZ);
	Chunk* chunk = world->chunks[index];

	if (chunk == NULL)
	{
		chunk = Calloc(Chunk);
		chunk->cPos = ivec3(cX, 0, cZ);
		chunk->wPos = ivec3(cX * CHUNK_SIZE, 0.0f, cZ * CHUNK_SIZE);
		GenerateChunkTerrain(world->noise, chunk);
		world->chunks[index] = chunk;
		SetChunkLoaded(world, chunk);
	}

	return chunk;
}

inline Chunk* CreateChunk(World* world, ivec3 cPos)
{
	return CreateChunk(world, cPos.x, cPos.z);
}

inline bool BlockInsideWorld(World* world, int wX, int wY, int wZ)
{
	return wX >= 0 && wX < world->sizeInBlocks.x && wY >= 0 && wY < WORLD_BLOCK_HEIGHT && 
		wZ >= 0 && wZ < world->sizeInBlocks.z;
}

inline int GetBlock(Chunk* chunk, int lX, int lY, int lZ)
{
	return chunk->blocks[lX + CHUNK_SIZE * (lY + WORLD_BLOCK_HEIGHT * lZ)];
}

static int GetBlock(World* world, int wX, int wY, int wZ)
{
	if (!BlockInsideWorld(world, wX, wY, wZ)) return 0;

	ivec3 cPos = ToChunkPos(wX, wZ);
	Chunk* chunk = GetChunk(world, cPos);

	if (chunk == NULL) 
	{
		chunk = CreateChunk(world, cPos);
		Assert(chunk != NULL);
	}

	return GetBlock(chunk, wX & (CHUNK_SIZE - 1), wY, wZ & (CHUNK_SIZE - 1));
}

inline int GetBlock(World* world, ivec3 pos)
{
	return GetBlock(world, pos.x, pos.y, pos.z);
}

static void BuildBlock(World* world, Chunk* chunk, float x, float y, float z, ivec3 wPos)
{
	Mesh* mesh = &chunk->mesh;

	// Top face.
	if (GetBlock(world, wPos.x, wPos.y + 1, wPos.z) == 0)
	{
		SetIndices(mesh);
		SetVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Bottom face.
	if (GetBlock(world, wPos.x, wPos.y - 1, wPos.z) == 0)
	{
		SetIndices(mesh);
		SetVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Front face.
	if (GetBlock(world, wPos.x, wPos.y, wPos.z + 1) == 0)
	{
		SetIndices(mesh);
		SetVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); 
		SetVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Back face.
	if (GetBlock(world, wPos.x, wPos.y, wPos.z - 1) == 0)
	{
		SetIndices(mesh);
		SetVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Right face.
	if (GetBlock(world, wPos.x + 1, wPos.y, wPos.z) == 0)
	{
		SetIndices(mesh);
		SetVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Left face.
	if (GetBlock(world, wPos.x - 1, wPos.y, wPos.z) == 0)
	{
		SetIndices(mesh);
		SetVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}

static void BuildChunk(World* world, Chunk* chunk)
{
	InitializeMesh(&chunk->mesh);

	for (int y = 0; y < WORLD_BLOCK_HEIGHT; y++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				int block = GetBlock(chunk, x, y, z);

				if (block != 0)
					BuildBlock(world, chunk, (float)x, (float)y, (float)z, chunk->wPos + ivec3(x, y, z));
			}
		}
	}
	
	FillMeshData(&chunk->mesh);
	chunk->state = CHUNK_BUILT;
}

static void DestroyChunk(World* world, Chunk* chunk, int i)
{
	if (chunk->state == CHUNK_BUILT)
		DestroyMesh(&chunk->mesh);

	ivec3 pos = chunk->cPos;
   	int index = ChunkIndex(world, pos.x, pos.z);
  	world->chunks[index] = NULL;
	SetChunkUnloaded(world, chunk, i);
	
	free(chunk);
}

static void LoadSurroundingChunks(World* world, ivec3 pos)
{
	if (pos != world->lastLoadPos)
	{
		ivec3 max = world->sizeInChunks;

		int minX = Max(pos.x - world->loadRangeH, 0);
		int maxX = Min(pos.x + world->loadRangeH, max.x - 1);
		int minZ = Max(pos.z - world->loadRangeH, 0);
		int maxZ = Min(pos.z + world->loadRangeH, max.z - 1);

		for (int x = minX; x <= maxX; x++)
		{
			for (int z = minZ; z <= maxZ; z++)
			{
				Chunk* chunk = CreateChunk(world, x, z);
				Assert(chunk != NULL);

				if (chunk->state == CHUNK_GENERATED)
					BuildChunk(world, chunk);
			}
		}

		world->lastLoadPos = pos;
	}
}

static void UnloadChunks(World* world, ivec3 pos)
{
	for (int i = 0; i < world->loadedCount; i++)
	{
		Chunk* chunk = world->loadedChunks[i];

  		int diffX = abs(chunk->cPos.x - pos.x);
		int diffZ = abs(chunk->cPos.z - pos.z);

		int lim = world->loadRangeH + 1;
		
		if (diffX > lim || diffZ > lim)
		{
			DestroyChunk(world, chunk, i);
			i--;
		}
	}
}

inline void DrawChunk(Chunk* chunk)
{
	DrawMesh(&chunk->mesh);
}
