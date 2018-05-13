// Voxel Engine
// Jason Bricco

inline void UpdateChunk(World* world, Chunk* chunk);

inline void EnqueueChunk(ChunkQueue& queue, Chunk* chunk)
{
    if (queue.front == NULL)
        queue.front = chunk;
    else queue.end->next = chunk;

    queue.end = chunk;
    queue.count++;
}

inline Chunk* DequeueChunk(ChunkQueue& queue)
{
    Chunk* front = queue.front;
    queue.front = front->next;
    front->next = NULL;
    queue.count--;
    return front;
}

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

inline void SetBlockPadded(Chunk* chunk, int rX, int rY, int rZ, int block)
{
    int index = rX + PADDED_CHUNK_SIZE * (rY + PADDED_CHUNK_SIZE * rZ);
    Assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

inline void SetBlockPadded(Chunk* chunk, RelPos pos, int block)
{
    SetBlockPadded(chunk, pos.x, pos.y, pos.z, block);
}

inline void SetBlock(Chunk* chunk, int rX, int rY, int rZ, int block)
{
    SetBlockPadded(chunk, rX + 1, rY + 1, rZ + 1, block);
}

inline void SetBlock(Chunk* chunk, RelPos pos, int block)
{
	SetBlock(chunk, pos.x, pos.y, pos.z, block);
}

inline void SetBlock(World* world, LWorldPos wPos, int block)
{
	LChunkPos cPos = ToChunkPos(wPos);
	Chunk* chunk = GetChunk(world, cPos);
	Assert(chunk != NULL);

	RelPos local = ToLocalPos(wPos);
	SetBlock(chunk, local, block);
    UpdateChunk(world, chunk);

    // Set this block to neighbor padding if it is on this chunk's edge.
    if (local.x == 0) 
    {
        chunk = GetChunk(world, cPos.x - 1, cPos.y, cPos.z);
        SetBlock(chunk, CHUNK_SIZE, local.y, local.z, block);
        UpdateChunk(world, chunk);
    }
    else if (local.x == CHUNK_SIZE - 1) 
    {
        chunk = GetChunk(world, cPos.x + 1, cPos.y, cPos.z);
        SetBlock(chunk, -1, local.y, local.z, block);
        UpdateChunk(world, chunk);
    }
    
    if (local.z == 0) 
    {
        chunk = GetChunk(world, cPos.x, cPos.y, cPos.z - 1);
        SetBlock(chunk, local.x, local.y, CHUNK_SIZE, block);
        UpdateChunk(world, chunk);
    }
    else if (local.z == CHUNK_SIZE - 1)
    {
        chunk = GetChunk(world, cPos.x, cPos.y, cPos.z + 1);
        SetBlock(chunk, local.x, local.y, -1, block);
        UpdateChunk(world, chunk);
    }

    if (local.y == 0)
    {
        chunk = GetChunk(world, cPos.x, cPos.y - 1, cPos.z);
        SetBlock(chunk, local.x, CHUNK_SIZE, local.z, block);
        UpdateChunk(world, chunk);
    }
    else if (local.y == CHUNK_SIZE - 1)
    {
        chunk = GetChunk(world, cPos.x, cPos.y + 1, cPos.z);
        SetBlock(chunk, local.x, -1, local.z, block);
        UpdateChunk(world, chunk);
    }
}

inline void SetBlock(World* world, int lwX, int lwY, int lwZ, int block)
{
	SetBlock(world, ivec3(lwX, lwY, lwZ), block);
}

inline int GetBlockPadded(Chunk* chunk, int rX, int rY, int rZ)
{
    int index = rX + PADDED_CHUNK_SIZE * (rY + PADDED_CHUNK_SIZE * rZ);
    Assert(index >= 0 && index < CHUNK_SIZE_3);
    return chunk->blocks[index];;
}

inline int GetBlockPadded(Chunk* chunk, RelPos pos)
{
    return GetBlockPadded(chunk, pos.x, pos.y, pos.z);
}

inline int GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
    return GetBlockPadded(chunk, rX + 1, rY + 1, rZ + 1);
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

inline float GetNoiseValue2D(float* noiseSet, int x, int z)
{
	return (noiseSet[x * (PADDED_CHUNK_SIZE) + z] + 1.0f) / 2.0f;
}

inline float GetRawNoiseValue2D(float* noiseSet, int x, int z)
{
    return noiseSet[x * (PADDED_CHUNK_SIZE) + z];
}

inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z)
{
	return (noiseSet[z + PADDED_CHUNK_SIZE * (y + PADDED_CHUNK_SIZE * x)] + 1.0f) / 2.0f;
}

inline float* GetNoise2D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = PADDED_CHUNK_SIZE;
    int sizeY = 1;
    int sizeZ = PADDED_CHUNK_SIZE;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

inline float* GetNoise3D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = PADDED_CHUNK_SIZE;
    int sizeY = PADDED_CHUNK_SIZE;
    int sizeZ = PADDED_CHUNK_SIZE;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static void GenerateChunkTerrain(World* world, Chunk* chunk)
{
    WorldPos start = chunk->cPos * CHUNK_SIZE;
    start -= 1;

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->seed);

	noise->SetFrequency(0.015f);
	noise->SetFractalOctaves(4);
	noise->SetFractalType(Noise::RigidMulti);
	float* ridged = GetNoise2D(noise, Noise::SimplexFractal, start.x, 0, start.z, 0.5f);

	noise->SetFrequency(0.025f);
	noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, Noise::SimplexFractal, start.x, 0, start.z, 0.5f);

	noise->SetFrequency(0.01f);
    float* biome = GetNoise2D(noise, Noise::Simplex, start.x, 0, start.z);

	noise->SetFractalOctaves(2);
	noise->SetFrequency(0.015f);
	noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, Noise::SimplexFractal, start.x, start.y, start.z, 0.2f);

	for (int x = 0; x < PADDED_CHUNK_SIZE; x++)
    {
        for (int z = 0; z < PADDED_CHUNK_SIZE; z++)
        {
        	float terrainVal;
        	float biomeVal = GetRawNoiseValue2D(biome, x, z);

        	// Value for flat terrain.
        	float flat = GetNoiseValue2D(base, x, z);
        	flat = ((flat * 0.2f) * 30.0f) + 10.0f;

        	// Value for mountainous terrain.
        	float mountain = GetNoiseValue2D(ridged, x, z);
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

        	for (int y = 0; y < PADDED_CHUNK_SIZE; y++)
        	{
        		int wY = start.y + y;
        		float compVal = GetNoiseValue3D(comp, x, y, z);

        		if (wY <= height - 10)
        		{
	        		if (compVal <= 0.2f)
	        		{
	        			SetBlockPadded(chunk, x, y, z, BLOCK_STONE);
	        			continue;
	        		}
	        	}

        		if (wY == height)
        			SetBlockPadded(chunk, x, y, z, BLOCK_GRASS);
        		else if (wY > height && wY <= SEA_LEVEL)
        			SetBlockPadded(chunk, x, y, z, BLOCK_WATER);
        		else 
                {
                    if (wY < height)
                        SetBlockPadded(chunk, x, y, z, BLOCK_DIRT);
                }
        	}
        }
    }

    Noise::FreeNoiseSet(ridged);
    Noise::FreeNoiseSet(base);
    Noise::FreeNoiseSet(biome);
    Noise::FreeNoiseSet(comp);

    chunk->state = CHUNK_GENERATED;
    delete noise;
}

// Fill a chunk with a single block type.
static void FillChunk(Chunk* chunk, int block)
{
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int x = 0; x < CHUNK_SIZE; x++)
                SetBlock(chunk, x, y, z, block);
        }
    }
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
        return Calloc(Chunk, sizeof(Chunk), "Chunk");

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
        chunk->active = true;
        QueueAsync(GenerateChunkTerrain, world, chunk);
		world->chunks[index] = chunk;
	}

	return chunk;
}

static void DestroyChunk(World* world, Chunk* chunk)
{
	DestroyMesh(chunk->mesh);
    chunk->mesh = NULL;
	AddChunkToPool(world, chunk);
}

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
inline void BuildBlock(World* world, Chunk* chunk, int xi, int yi, int zi, int block)
{
    Mesh* mesh = chunk->mesh;
	float* textures = world->blockData[block].textures;

    float x = (float)xi, y = (float)yi, z = (float)zi;

	// Top face.
	if (GetBlock(chunk, xi, yi + 1, zi) == BLOCK_AIR)
	{
		float tex = textures[FACE_TOP];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Bottom face.
	if (GetBlock(chunk, xi, yi - 1, zi) == BLOCK_AIR)
	{
		float tex = textures[FACE_BOTTOM];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Front face.
	if (GetBlock(chunk, xi, yi, zi + 1) == BLOCK_AIR)
	{
		float tex = textures[FACE_FRONT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f); 
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Back face.
	if (GetBlock(chunk, xi, yi, zi - 1) == BLOCK_AIR)
	{
		float tex = textures[FACE_BACK];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Right face.
	if (GetBlock(chunk, xi + 1, yi, zi) == BLOCK_AIR)
	{
		float tex = textures[FACE_RIGHT];
		SetMeshIndices(mesh);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
		SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Left face.
	if (GetBlock(chunk, xi - 1, yi, zi) == BLOCK_AIR)
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
    chunk->mesh = CreateMesh();

	for (int y = 0; y < CHUNK_SIZE; y++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				int block = GetBlock(chunk, x, y, z);

				if (block != BLOCK_AIR)
					BuildBlock(world, chunk, x, y, z, block);
			}
		}
	}
	
    chunk->state = CHUNK_NEEDS_FILL;
}

// Rebuilds chunk meshes.
inline void UpdateChunk(World* world, Chunk* chunk)
{
	if (chunk->state == CHUNK_BUILT)
    {
        DestroyMesh(chunk->mesh);
        BuildChunk(world, chunk);
    }
}

// To allow "infinite" terrain, the world is always located near the origin.
// This function fills the world near the origin based on the reference
// world position within the world.
static void ShiftWorld(World* world)
{
    // Return all chunks in the active area to the hash table.
	for (int i = 0; i < world->totalChunks; i++)
	{
		AddChunkToHash(world, world->chunks[i]);
		world->chunks[i] = NULL;
	}

    // Any existing chunks that still belong in the active area will be pulled in to their
    // new position. Any that don't exist in the hash table will be created.
	for (int z = 0; z < world->sizeH; z++)
	{
		for (int y = 0; y < world->sizeV; y++)
		{
			for (int x = 0; x < world->sizeH; x++)
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
            EnqueueChunk(world->destroyQueue, chunk);

        world->chunkHash[c] = NULL;
	}
}

static World* NewWorld(int loadRangeH, int loadRangeV)
{
	World* world = Calloc(World, sizeof(World), "World");

	// Load range worth of chunks on each side plus the middle chunk.
	world->sizeH = (loadRangeH * 2) + 1;
	world->sizeV = (loadRangeV * 2) + 1;

	world->spawnChunk = ivec3(0, 1, 0);

	world->totalChunks = Square(world->sizeH) * world->sizeV;
	world->chunks = Calloc(Chunk*, world->totalChunks * sizeof(Chunk*), "Chunks");

	ivec3 ref;
	ref.x = world->spawnChunk.x - loadRangeH;
	ref.y = world->spawnChunk.y - loadRangeV;
	ref.z = world->spawnChunk.z - loadRangeH;
	world->ref = ref;

	// Allocate extra chunks for the pool for world shifting. We create new chunks
	// before we destroy the old ones.
	int targetPoolSize = world->totalChunks * 2;
	world->pool = Calloc(Chunk*, targetPoolSize * sizeof(Chunk*), "ChunkPool");
	world->poolSize = 0;
    world->maxPoolSize = targetPoolSize;

	for (int i = 0; i < targetPoolSize; i++)
    {
        Chunk* chunk = Malloc(Chunk, sizeof(Chunk), "Chunk");
		AddChunkToPool(world, chunk);
    }

	float minH = (float)(loadRangeH * CHUNK_SIZE);
	float maxH = minH + CHUNK_SIZE;

	float minV = (float)(loadRangeV * CHUNK_SIZE);
	float maxV = minV + CHUNK_SIZE;

	world->pBounds = NewRect(vec3(minH, minV, minH), vec3(maxH, maxV, maxH));

	CreateBlockData(world->blockData);

    srand((uint32_t)time(0));
    world->seed = rand();

	ShiftWorld(world);

	return world;
}

static void CheckWorld(World* world, Player* player)
{
    Rectf bounds = world->pBounds;
    vec3 pos = player->pos;
    bool shift = false;

    Assert(player->pos.x >= 0.0f);
    Assert(player->pos.y >= 0.0f);
    Assert(player->pos.z >= 0.0f);

    while (pos.x < bounds.min.x) 
    {
        pos.x = bounds.max.x - (bounds.min.x - pos.x);
        world->ref.x--;
        shift = true;
    }
    
    while (pos.x > bounds.max.x) 
    {
        pos.x = bounds.min.x + (pos.x - bounds.max.x);
        world->ref.x++;
        shift = true;
    }

    while (pos.y < bounds.min.y)
    {
        pos.y = bounds.max.y - (bounds.min.y - pos.y);
        world->ref.y--;
        shift = true;
    }
    
    while (pos.y > bounds.max.y)
    {
        pos.y = bounds.min.y + (pos.y - bounds.max.y);
        world->ref.y++;
        shift = true;
    }

    while (pos.z < bounds.min.z) 
    {
        pos.z = bounds.max.z - (bounds.min.z - pos.z);
        world->ref.z--;
        shift = true;
    }
    
    while (pos.z > bounds.max.z) 
    {
        pos.z = bounds.min.z + (pos.z - bounds.max.z);
        world->ref.z++;
        shift = true;
    }

    if (shift) 
    {
        player->pos = pos;
        ShiftWorld(world);
    }
}

static void TryBuildMeshes(World* world)
{
    for (int z = 0; z < world->sizeH; z++)
    {
        for (int y = 0; y < world->sizeV; y++)
        {
            for (int x = 0; x < world->sizeH; x++)
            {
                Chunk* chunk = GetChunk(world, x, y, z);

                switch (chunk->state)
                {
                    case CHUNK_GENERATED:
                    {
                        chunk->state = CHUNK_BUILDING;
                        QueueAsync(BuildChunk, world, chunk);
                        break;
                    }

                    case CHUNK_NEEDS_FILL:
                    {
                        FillMeshData(chunk->mesh);
                        chunk->state = CHUNK_BUILT;
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }
}

static void UpdateWorld(World* world, Player* player)
{
    CheckWorld(world, player);
    TryBuildMeshes(world);

    while (world->destroyQueue.count > 0)
    {
        Chunk* chunk = DequeueChunk(world->destroyQueue);

        if (chunk->state != CHUNK_GENERATING && chunk->state != CHUNK_BUILDING)
            DestroyChunk(world, chunk);
        else EnqueueChunk(world->destroyQueue, chunk);
    }
}
