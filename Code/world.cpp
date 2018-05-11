// Voxel Engine
// Jason Bricco

inline void UpdateChunk(World* world, Chunk* chunk, ivec3 lPos);

inline RelPos ToLocalPos(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX & CHUNK_SIZE - 1, lwY & CHUNK_SIZE - 1, lwZ & CHUNK_SIZE - 1);
}

inline RelPos ToLocalPos(LWorldPos wPos)
{
	return ToLocalPos(wPos.x, wPos.y, wPos.z);
}

inline LChunkPos ToChunkPos(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX >> CHUNK_SIZE_BITS, lwY >> CHUNK_SIZE_BITS, lwZ >> CHUNK_SIZE_BITS);
}

inline LChunkPos ToChunkPos(LWorldPos pos)
{
	return ToChunkPos(pos.x, pos.y, pos.z);
}

inline ivec3 ToChunkPos(vec3 wPos)
{
	return ToChunkPos((int)wPos.x, (int)wPos.y, (int)wPos.z);
}

inline int ChunkIndex(World* world, int lcX, int lcY, int lcZ)
{
	return lcX + world->sizeH * (lcY + world->sizeV * lcZ);
}

inline Chunk* GetChunk(World* world, int lcX, int lcY, int lcZ)
{
	return world->chunks[ChunkIndex(world, lcX, lcY, lcZ)];
}

inline Chunk* GetChunk(World* world, LChunkPos pos)
{
	return GetChunk(world, pos.x, pos.y, pos.z);
}

inline uint32_t ChunkHashBucket(ivec3 wPos)
{
	uint32_t hashValue = 7919 * wPos.y + (31 + wPos.x) * 23 + wPos.z;
	return hashValue & (CHUNK_HASH_SIZE - 1);
}

// The chunk pool stores existing chunks during a world shift. Chunks will be
// pulled out from the pool to fill the new world section if applicable.
// Otherwise, new chunks will be created and the ones remaining in the pool
// will be destroyed.
static Chunk* ChunkFromHash(World* world, uint32_t bucket, ChunkPos cPos)
{
    Chunk* chunk = world->chunkHash[bucket];

    if (chunk == NULL) 
        return NULL;

    if (chunk->cPos == cPos)
    {
        chunk->active = true;
        return chunk;
    }

    uint32_t firstBucket = bucket;

    while (true)
    {
        bucket = (bucket + 1) & (CHUNK_HASH_SIZE - 1);

        if (bucket == firstBucket)
            return NULL;

        chunk = world->chunkHash[bucket];

        if (chunk != NULL && chunk->cPos == cPos)
        {
            chunk->active = true;
            return chunk;
        }
    }
}

inline Chunk* ChunkFromHash(World* world, int32_t wX, int32_t wY, int32_t wZ)
{
	ivec3 wPos = ivec3(wX, wY, wZ);
	return ChunkFromHash(world, ChunkHashBucket(wPos), wPos);
}

inline void AddChunkToHash(World* world, Chunk* chunk)
{
	if (chunk == NULL) return;

	uint32_t bucket = ChunkHashBucket(chunk->cPos);

    #if ASSERTIONS
    uint32_t firstBucket = bucket;
    #endif

    while (world->chunkHash[bucket] != NULL)
    {
        bucket = (bucket + 1) & (CHUNK_HASH_SIZE - 1);
        Assert(bucket != firstBucket);
    }

    chunk->active = false;
    world->chunkHash[bucket] = chunk;
}

inline void SetBlock(Chunk* chunk, int lX, int lY, int lZ, int block)
{
	chunk->blocks[lX + CHUNK_SIZE * (lY + CHUNK_SIZE * lZ)] = block;
}

inline void SetBlock(Chunk* chunk, ivec3 lPos, int block)
{
	SetBlock(chunk, lPos.x, lPos.y, lPos.z, block);
}

inline void SetBlock(World* world, ivec3 wPos, int block, bool update)
{
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

inline int GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
	return chunk->blocks[rX + CHUNK_SIZE * (rY + CHUNK_SIZE * rZ)];
}

inline int GetBlock(Chunk* chunk, RelPos pos)
{
	return GetBlock(chunk, pos.x, pos.y, pos.z);
}

static int GetBlock(World* world, int lwX, int lwY, int lwZ)
{
	LChunkPos lcPos = ToChunkPos(lwX, lwY, lwZ);
	Chunk* chunk = GetChunk(world, lcPos);
	Assert(chunk != NULL);

	RelPos rPos = ToLocalPos(lwX, lwY, lwZ);
	return GetBlock(chunk, rPos);
}

inline int GetBlock(World* world, LWorldPos pos)
{
	return GetBlock(world, pos.x, pos.y, pos.z);
}

inline float GetNoiseValue2D(float* noiseSet, int index)
{
	return (noiseSet[index] + 1.0f) / 2.0f;
}

inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z)
{
	return (noiseSet[z + CHUNK_SIZE * (y + CHUNK_SIZE * x)] + 1.0f) / 2.0f;
}

static void GenerateChunkTerrain(World* world, Chunk* chunk)
{
	BEGIN_TIMED_BLOCK(GEN_TERRAIN);

    WorldPos start = chunk->cPos * CHUNK_SIZE;

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->seed);

	noise->SetFrequency(0.015f);
	noise->SetFractalOctaves(4);
	noise->SetFractalType(Noise::RigidMulti);
	float* ridged = noise->GetSimplexFractalSet(start.x, 0, start.z, CHUNK_SIZE, 1, CHUNK_SIZE, 0.5f); 

	noise->SetFrequency(0.025f);
	noise->SetFractalType(Noise::Billow);
	float* base = noise->GetSimplexFractalSet(start.x, 0, start.z, CHUNK_SIZE, 1, CHUNK_SIZE, 0.5f);

	noise->SetFrequency(0.01f);
	float* biome = noise->GetSimplexSet(start.x, 0, start.z, CHUNK_SIZE, 1, CHUNK_SIZE);

	noise->SetFractalOctaves(2);
	noise->SetFrequency(0.015f);
	noise->SetFractalType(Noise::FBM);
	float* comp = noise->GetSimplexFractalSet(start.x, start.y, start.z, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.2f);

	int index = 0;

	for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
        	float terrainVal;
        	float biomeVal = biome[index];

        	// Value for flat terrain.
        	float flat = GetNoiseValue2D(base, index);
        	flat = ((flat * 0.2f) * 30.0f) + 10.0f;

        	// Value for mountainous terrain.
        	float mountain = GetNoiseValue2D(ridged, index);
        	mountain = (pow(mountain, 3.5f) * 60.0f) + 20.0f;

        	float lower = 0.0f;
        	float upper = 0.6f;

        	if (biomeVal < lower)
        		terrainVal = flat;
        	else if (biomeVal > upper)
        		terrainVal = mountain;
        	else
        	{
        		// If we're close to the boundary between the two terrain types,
        		// interpolate between them for a smooth transition.
        		float a = SCurve3((biomeVal - lower) / (upper - lower));
        		terrainVal = Lerp(flat, mountain, a);
        	}

        	int height = (int)terrainVal;
        	index++;

        	for (int y = 0; y < CHUNK_SIZE; y++)
        	{
        		int wY = start.y + y;
        		float compVal = GetNoiseValue3D(comp, x, y, z);

        		if (wY <= height - 10)
        		{
	        		if (compVal <= 0.2f)
	        		{
	        			SetBlock(chunk, x, y, z, BLOCK_STONE);
	        			continue;
	        		}
	        	}

        		if (wY == height) 
        			SetBlock(chunk, x, y, z, BLOCK_GRASS);
        		else if (wY > height && wY <= SEA_LEVEL)
        			SetBlock(chunk, x, y, z, BLOCK_WATER);
        		else 
                {
                    if (wY < height)
                        SetBlock(chunk, x, y, z, BLOCK_DIRT);
                }
        	}
        }
    }

    Noise::FreeNoiseSet(ridged);
    Noise::FreeNoiseSet(base);
    Noise::FreeNoiseSet(biome);
    Noise::FreeNoiseSet(comp);

    delete noise;

    static int count = 0;
    count++;
    char buffer[16];
    sprintf(buffer, "%i\n", count);
    OutputDebugString(buffer);

	END_TIMED_BLOCK(GEN_TERRAIN);
}

// Fill a chunk with a single block type.
static void FillChunk(Chunk* chunk, int block)
{
	for (int i = 0; i < CHUNK_SIZE_3; i++)
		chunk->blocks[i] = block;
}

inline void AddChunkToPool(World* world, Chunk* chunk)
{
    if (world->poolSize + 1 > world->maxPoolSize)
    {
        int newMax = world->maxPoolSize + (world->totalChunks / 2);
        world->pool = (Chunk**)realloc(world->pool, newMax * sizeof(Chunk*));
        world->maxPoolSize = newMax;
    }

	memset(chunk, 0, sizeof(Chunk));
	world->pool[world->poolSize++] = chunk;
}

inline Chunk* ChunkFromPool(World* world)
{
	if (world->poolSize == 0)
        return Calloc(Chunk);

	Chunk* chunk = world->pool[world->poolSize - 1];
	world->poolSize--;
	return chunk;
}

static Chunk* CreateChunk(World* world, int cX, int cY, int cZ, ChunkPos cPos)
{
    int index = ChunkIndex(world, cX, cY, cZ);
	Chunk* chunk = world->chunks[index];

	if (chunk == NULL)
	{
		chunk = ChunkFromPool(world);
		chunk->lcPos = ivec3(cX, cY, cZ);
		chunk->cPos = cPos;
        chunk->lwPos = chunk->lcPos * CHUNK_SIZE;
        QueueAsync(GenerateChunkTerrain, world, chunk);
		world->chunks[index] = chunk;
	}

	return chunk;
}

static void DestroyChunk(World* world, Chunk* chunk)
{
	if (chunk->state == CHUNK_BUILT)
		DestroyMesh(chunk->mesh);

	AddChunkToPool(world, chunk);
}

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
inline void BuildBlock(World* world, Mesh* mesh, float x, float y, float z, LWorldPos pos, int block)
{
	float* textures = world->blockData[block].textures;

	// Top face.
	if (GetBlock(world, pos.x, pos.y + 1, pos.z) == BLOCK_AIR)
	{
		float tex = textures[FACE_TOP];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Bottom face.
	if (GetBlock(world, pos.x, pos.y - 1, pos.z) == BLOCK_AIR)
	{
		float tex = textures[FACE_BOTTOM];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Front face.
	if (GetBlock(world, pos.x, pos.y, pos.z + 1) == BLOCK_AIR)
	{
		float tex = textures[FACE_FRONT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f); 
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Back face.
	if (GetBlock(world, pos.x, pos.y, pos.z - 1) == BLOCK_AIR)
	{
		float tex = textures[FACE_BACK];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Right face.
	if (GetBlock(world, pos.x + 1, pos.y, pos.z) == BLOCK_AIR)
	{
		float tex = textures[FACE_RIGHT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Left face.
	if (GetBlock(world, pos.x - 1, pos.y, pos.z) == BLOCK_AIR)
	{
		float tex = textures[FACE_LEFT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}

// Builds mesh data for the chunk.
static void BuildChunk(World* world, Chunk* chunk)
{
	BEGIN_TIMED_BLOCK(BUILD_CHUNK);

	chunk->mesh = CreateMesh();

	for (int y = 0; y < CHUNK_SIZE; y++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				int block = GetBlock(chunk, x, y, z);

				if (block != BLOCK_AIR)
				{
                    LWorldPos pos = chunk->lwPos + ivec3(x, y, z);
					BuildBlock(world, chunk->mesh, (float)x, (float)y, (float)z, pos, block);
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

// Rebuilds chunk meshes.
inline void UpdateChunk(World* world, Chunk* chunk, ivec3 lPos)
{
	UpdateChunkDirect(world, chunk);

	ivec3 cP = chunk->lcPos;

	if (lPos.x == 0) UpdateChunkDirect(world, GetChunk(world, cP.x - 1, cP.y, cP.z));
	else if (lPos.x == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cP.x + 1, cP.y, cP.z));
	
	if (lPos.z == 0) UpdateChunkDirect(world, GetChunk(world, cP.x,cP.y, cP.z - 1));
	else if (lPos.z == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cP.x, cP.y, cP.z + 1));

	if (lPos.y == 0) UpdateChunkDirect(world, GetChunk(world, cP.x, cP.y - 1, cP.z));
	else if (lPos.y == CHUNK_SIZE - 1) UpdateChunkDirect(world, GetChunk(world, cP.x, cP.y + 1, cP.z));
}

// To allow "infinite" terrain, the world is always located near the origin.
// This function fills the world near the origin based on the reference
// world position within the world.
static void ShiftWorld(World* world)
{
	int edgeH = world->sizeH - 1;
	int edgeV = world->sizeV - 1;

    // Return all chunks in the active area to the hash table.
	for (int i = 0; i < world->totalChunks; i++)
	{
		AddChunkToHash(world, world->chunks[i]);
		world->chunks[i] = NULL;
	}

    // Any existing chunks that still belong in the active area will be pulled in to their
    // new position. Any that don't exist in the hash table will be created.
	for (int z = 0; z <= edgeH; z++)
	{
		for (int y = 0; y <= edgeV; y++)
		{
			for (int x = 0; x <= edgeH; x++)
			{
 				int wX = world->ref.x + x;
				int wY = world->ref.y + y;
				int wZ = world->ref.z + z;

				Chunk* chunk = ChunkFromHash(world, wX, wY, wZ);

				if (chunk == NULL)
					CreateChunk(world, x, y, z, ivec3(wX, wY, wZ));
				else 
				{
					chunk->lcPos = ivec3(x, y, z);
                    chunk->lwPos = chunk->lcPos * CHUNK_SIZE;
					world->chunks[ChunkIndex(world, x, y, z)] = chunk;
				}
			}
		}
	}

    // Any remaining chunks in the hash table are outside of the loaded area range
    // and should be returned to the pool.
	for (int c = 0; c < CHUNK_HASH_SIZE; c++)
	{
		Chunk* chunk = world->chunkHash[c];

        if (chunk != NULL && !chunk->active)
            DestroyChunk(world, chunk);

        world->chunkHash[c] = NULL;
	}

    // Build meshes for all non-boundary chunks.
	for (int z = 1; z < edgeH; z++)
	{
		for (int y = 1; y < edgeV; y++)
		{
			for (int x = 1; x < edgeH; x++)
			{
				Chunk* chunk = GetChunk(world, x, y, z);

				if (chunk->state == CHUNK_GENERATED)
					BuildChunk(world, chunk);
			}
		}
	}
}

static World* NewWorld(int loadRangeH, int loadRangeV)
{
	World* world = Calloc(World);

	// Load range worth of chunks on each side, the middle chunk, and two boundary
	// chunks form the total width.
	world->sizeH = (loadRangeH * 2) + 3;
	world->sizeV = (loadRangeV * 2) + 3;

	world->spawnChunk = ivec3(0, 1, 0);

	world->totalChunks = Square(world->sizeH) * world->sizeV;
	world->chunks = (Chunk**)calloc(1, world->totalChunks * sizeof(Chunk*));

	ivec3 ref;
	ref.x = world->spawnChunk.x - loadRangeH - 1;
	ref.y = world->spawnChunk.y - loadRangeV - 1;
	ref.z = world->spawnChunk.z - loadRangeH - 1;
	world->ref = ref;

	// Allocate extra chunks for the pool for world shifting. We create new chunks
	// before we destroy the old ones.
	int targetPoolSize = world->totalChunks * 2;
	world->pool = (Chunk**)calloc(1, targetPoolSize * sizeof(Chunk*));
	world->poolSize = 0;
    world->maxPoolSize = targetPoolSize;

	for (int i = 0; i < targetPoolSize; i++)
		AddChunkToPool(world, Malloc(Chunk));

	float minH = (float)((loadRangeH + 1) * CHUNK_SIZE);
	float maxH = minH + CHUNK_SIZE;

	float minV = (float)((loadRangeV + 1) * CHUNK_SIZE);
	float maxV = minV + CHUNK_SIZE;

	world->pBounds = NewRect(vec3(minH, minV, minH), vec3(maxH, maxV, maxH));

	CreateBlockData(world->blockData);

    srand((uint32_t)time(0));
    world->seed = rand();

	ShiftWorld(world);

	return world;
}
