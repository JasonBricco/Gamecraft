//
// Jason Bricco
//

static BlockData g_blockData[BLOCK_COUNT];

static inline void EnqueueChunk(ChunkQueue& queue, Chunk* chunk)
{
    if (queue.front == nullptr)
        queue.front = chunk;
    else queue.end->next = chunk;

    queue.end = chunk;
    queue.count++;
}

static inline Chunk* DequeueChunk(ChunkQueue& queue)
{
    Chunk* front = queue.front;
    queue.front = front->next;
    front->next = nullptr;
    queue.count--;
    return front;
}

static inline RelPos ToLocalPos(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX & CHUNK_SIZE - 1, lwY & CHUNK_SIZE - 1, lwZ & CHUNK_SIZE - 1);
}

static inline RelPos ToLocalPos(LWorldPos wPos)
{
	return ToLocalPos(wPos.x, wPos.y, wPos.z);
}

static inline LChunkPos ToChunkPos(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX >> CHUNK_SIZE_BITS, lwY >> CHUNK_SIZE_BITS, lwZ >> CHUNK_SIZE_BITS);
}

static inline LChunkPos ToChunkPos(LWorldPos pos)
{
	return ToChunkPos(pos.x, pos.y, pos.z);
}

static inline ivec3 ToChunkPos(vec3 wPos)
{
	return ToChunkPos((int)wPos.x, (int)wPos.y, (int)wPos.z);
}

static inline int ChunkIndex(World* world, int lcX, int lcY, int lcZ)
{
	return lcX + world->sizeH * (lcY + world->sizeV * lcZ);
}

static inline Chunk* GetChunk(World* world, int lcX, int lcY, int lcZ)
{
	return world->chunks[ChunkIndex(world, lcX, lcY, lcZ)];
}

static inline Chunk* GetChunk(World* world, LChunkPos pos)
{
	return GetChunk(world, pos.x, pos.y, pos.z);
}

static inline uint32_t ChunkHashBucket(ivec3 wPos)
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

    if (chunk == nullptr) 
        return nullptr;

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
            return nullptr;

        chunk = world->chunkHash[bucket];

        if (chunk != nullptr && chunk->cPos == cPos)
        {
            chunk->active = true;
            return chunk;
        }
    }
}

static inline Chunk* ChunkFromHash(World* world, int32_t wX, int32_t wY, int32_t wZ)
{
	ivec3 wPos = ivec3(wX, wY, wZ);
	return ChunkFromHash(world, ChunkHashBucket(wPos), wPos);
}

static inline void AddChunkToHash(World* world, Chunk* chunk)
{
	if (chunk == nullptr) return;

	uint32_t bucket = ChunkHashBucket(chunk->cPos);

    #if _DEBUG
    uint32_t firstBucket = bucket;
    #endif

    while (world->chunkHash[bucket] != nullptr)
    {
        bucket = (bucket + 1) & (CHUNK_HASH_SIZE - 1);
        assert(bucket != firstBucket);
    }

    chunk->active = false;
    world->chunkHash[bucket] = chunk;
}

// Sets a block to the given chunk. Assumes the coordinates take into account the chunk
// padding and doesn't offset them.
static inline void SetBlockPadded(Chunk* chunk, int rX, int rY, int rZ, Block block)
{
    int index = rX + PADDED_CHUNK_SIZE * (rY + PADDED_CHUNK_SIZE * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

static inline void SetBlockPadded(Chunk* chunk, RelPos pos, Block block)
{
    SetBlockPadded(chunk, pos.x, pos.y, pos.z, block);
}

// Sets a block to the given chunk. Assumes the coordinates are in the range 
// 0 to CHUNK_SIZE and offsets them to account for padding.
static inline void SetBlock(Chunk* chunk, int rX, int rY, int rZ, Block block)
{
    SetBlockPadded(chunk, rX + 1, rY + 1, rZ + 1, block);
}

static inline void SetBlock(Chunk* chunk, RelPos pos, Block block)
{
	SetBlock(chunk, pos.x, pos.y, pos.z, block);
}

#define SET_TO_NEIGHBOR(lcX, lcY, lcZ, rX, rY, rZ) {\
    chunk = GetChunk(world, lcX, lcY, lcZ);\
    SetBlock(chunk, rX, rY, rZ, block);\
    chunk->state = CHUNK_UPDATE;\
    chunk->modified = true;\
}

// Sets a block to the given chunk. If blocks are on the edge of the chunk,
// the neighbor chunk's padding will be updated as well.
static inline void SetBlockAndUpdatePadding(World* world, Chunk* chunk, int rX, int rY, int rZ, Block block)
{
    SetBlock(chunk, rX, rY, rZ, block);
    chunk->state = CHUNK_UPDATE;
    chunk->modified = true;

    ivec3 dir = ivec3(0);
    ivec3 bP = ivec3(0);

    if (rX == 0)
    {
        dir.x = -1;
        bP.x = CHUNK_SIZE;
    }
    else if (rX == CHUNK_SIZE - 1)
    {
        dir.x = 1;
        bP.x = -1;
    }

    if (rY == 0)
    {
        dir.y = -1;
        bP.y = CHUNK_SIZE;
    }
    else if (rY == CHUNK_SIZE - 1)
    {
        dir.y = 1;
        bP.y = -1;
    }

    if (rZ == 0)
    {
        dir.z = -1;
        bP.z = CHUNK_SIZE;
    }
    else if (rZ == CHUNK_SIZE - 1)
    {
        dir.z = 1;
        bP.z = -1;
    }

    LChunkPos p = chunk->lcPos;

    if (dir.x != 0)
    {
        SET_TO_NEIGHBOR(p.x + dir.x, p.y, p.z, bP.x, rY, rZ);

        if (dir.y != 0) 
        {
            SET_TO_NEIGHBOR(p.x + dir.x, p.y + dir.y, p.x, bP.x, bP.y, rZ);

            if (dir.z != 0)
                SET_TO_NEIGHBOR(p.x + dir.x, p.y + dir.y, p.z + dir.z, bP.x, bP.y, bP.z);
        }

        if (dir.z != 0)
            SET_TO_NEIGHBOR(p.x + dir.x, p.y, p.z + dir.z, bP.x, rY, bP.z);
    }

    if (dir.y != 0)
        SET_TO_NEIGHBOR(p.x, p.y + dir.y, p.z, rX, bP.y, rZ);

    if (dir.z != 0)
    {
        SET_TO_NEIGHBOR(p.x, p.y, p.z + dir.z, rX, rY, bP.z);

        if (dir.y != 0)
            SET_TO_NEIGHBOR(p.x, p.y + dir.y, p.z + dir.z, rX, bP.y, bP.z);
    }
}

static inline void SetBlock(World* world, LWorldPos wPos, Block block)
{
	LChunkPos cPos = ToChunkPos(wPos);
	Chunk* chunk = GetChunk(world, cPos);
	assert(chunk != nullptr);

	RelPos local = ToLocalPos(wPos);
	SetBlockAndUpdatePadding(world, chunk, local.x, local.y, local.z, block);
}

static inline void SetBlock(World* world, int lwX, int lwY, int lwZ, Block block)
{
	SetBlock(world, ivec3(lwX, lwY, lwZ), block);
}

static inline Block GetBlockPadded(Chunk* chunk, int rX, int rY, int rZ)
{
    int index = rX + PADDED_CHUNK_SIZE * (rY + PADDED_CHUNK_SIZE * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    return chunk->blocks[index];;
}

static inline Block GetBlockPadded(Chunk* chunk, RelPos pos)
{
    return GetBlockPadded(chunk, pos.x, pos.y, pos.z);
}

static inline Block GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
    return GetBlockPadded(chunk, rX + 1, rY + 1, rZ + 1);
}

static inline Block GetBlock(Chunk* chunk, RelPos pos)
{
	return GetBlock(chunk, pos.x, pos.y, pos.z);
}

static Block GetBlock(World* world, int lwX, int lwY, int lwZ)
{
	LChunkPos lcPos = ToChunkPos(lwX, lwY, lwZ);
	Chunk* chunk = GetChunk(world, lcPos);
	assert(chunk != nullptr);

	RelPos rPos = ToLocalPos(lwX, lwY, lwZ);
	return GetBlock(chunk, rPos);
}

static inline Block GetBlock(World* world, LWorldPos pos)
{
	return GetBlock(world, pos.x, pos.y, pos.z);
}

static inline float GetNoiseValue2D(float* noiseSet, int x, int z)
{
	return (noiseSet[x * (PADDED_CHUNK_SIZE) + z] + 1.0f) / 2.0f;
}

static inline float GetRawNoiseValue2D(float* noiseSet, int x, int z)
{
    return noiseSet[x * (PADDED_CHUNK_SIZE) + z];
}

static inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z)
{
	return (noiseSet[z + PADDED_CHUNK_SIZE * (y + PADDED_CHUNK_SIZE * x)] + 1.0f) / 2.0f;
}

static inline float* GetNoise2D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = PADDED_CHUNK_SIZE;
    int sizeY = 1;
    int sizeZ = PADDED_CHUNK_SIZE;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static inline float* GetNoise3D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
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

    delete noise;
}

static void LoadChunk(World* world, Chunk* chunk)
{
    ChunkPos p = chunk->cPos;

    char path[MAX_PATH];
    sprintf(path, "%s\\%i%i%i.txt", PathToExe("Saves"), p.x, p.y, p.z);

    if (!PathFileExists(path))
        GenerateChunkTerrain(world, chunk);
    else
    {
        BEGIN_TIMED_BLOCK(READ_CREATE);

        uint16_t data[CHUNK_SIZE_3];

        END_TIMED_BLOCK(READ_CREATE);
        BEGIN_TIMED_BLOCK(READ);

        HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        assert(file != INVALID_HANDLE_VALUE);

        BOOL error = ReadFile(file, data, sizeof(uint16_t) * CHUNK_SIZE_3, NULL, NULL);
        CloseHandle(file);

        END_TIMED_BLOCK(READ);

        if (error)
        {
            Print("Failed to load chunk.");
            return;
        }

        BEGIN_TIMED_BLOCK(DECODE);

        int i = 0; 
        int loc = 0;

        while (i < CHUNK_SIZE_3)
        {
            uint16_t count = data[loc++];
            Block block = data[loc++];

            for (int j = 0; j < count; j++)
                chunk->blocks[i++] = block;
        }

        END_TIMED_BLOCK(DECODE);
    }

    chunk->state = CHUNK_GENERATED;
}

// Fill a chunk with a single block type.
static void FillChunk(World* world, Chunk* chunk, Block block)
{
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int x = 0; x < CHUNK_SIZE; x++)
                SetBlockAndUpdatePadding(world, chunk, x, y, z, block);
        }
    }
}

static inline void AddChunkToPool(World* world, Chunk* chunk)
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

static inline Chunk* ChunkFromPool(World* world)
{
	if (world->poolSize == 0)
        return Calloc<Chunk>();

	Chunk* chunk = world->pool[world->poolSize - 1];
	world->poolSize--;
	return chunk;
}

static Chunk* CreateChunk(World* world, int cX, int cY, int cZ, ChunkPos cPos)
{
    int index = ChunkIndex(world, cX, cY, cZ);
	Chunk* chunk = world->chunks[index];

	if (chunk == nullptr)
	{
		chunk = ChunkFromPool(world);
		chunk->lcPos = ivec3(cX, cY, cZ);
		chunk->cPos = cPos;
        chunk->lwPos = chunk->lcPos * CHUNK_SIZE;
        chunk->active = true;
        QueueAsync(LoadChunk, world, chunk);
		world->chunks[index] = chunk;
	}

	return chunk;
}

static inline void DestroyChunkMeshes(Chunk* chunk)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
    {
       DestroyMesh(chunk->meshes[i]);
       chunk->meshes[i] = nullptr;
    }
}

static void SaveChunk(Chunk* chunk)
{
    ChunkPos p = chunk->cPos;

    BEGIN_TIMED_BLOCK(BUFFER_CREATE);

    char path[MAX_PATH];
    sprintf(path, "%s\\%i%i%i.txt", PathToExe("Saves"), p.x, p.y, p.z);

    uint16_t data[CHUNK_SIZE_3];
    int loc = 0;

    END_TIMED_BLOCK(BUFFER_CREATE);
    BEGIN_TIMED_BLOCK(ENCODE);

    Block currentBlock = chunk->blocks[0];
    uint16_t count = 1;

    for (int i = 1; i < CHUNK_SIZE_3; i++)
    {
        Block block = chunk->blocks[i];

        if (block != currentBlock)
        {
            data[loc++] = count;
            data[loc++] = currentBlock;
            count = 1;
            currentBlock = block;
        }
        else count++;

        if (i == CHUNK_SIZE_3 - 1)
        {
            data[loc++] = count;
            data[loc++] = currentBlock;
        }
    }

    END_TIMED_BLOCK(ENCODE);
    BEGIN_TIMED_BLOCK(WRITE);

    HANDLE file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(file != INVALID_HANDLE_VALUE);

    DWORD bytesToWrite = sizeof(uint16_t) * loc;
    DWORD bytesWritten;

    BOOL error = WriteFile(file, data, bytesToWrite, &bytesWritten, NULL);
    assert(bytesWritten == bytesToWrite);

    CloseHandle(file);

    END_TIMED_BLOCK(WRITE);

    if (error) Print("Failed to save chunk.");
}

static void SaveWorld(World* world)
{
    char path[MAX_PATH];
    char* dir = PathToExe("Saves");
    CreateDirectory(dir, NULL);

    sprintf(path, "%s\\WorldData.txt", dir);

    WriteBinary(path, (char*)&world->seed, sizeof(int));

    for (int i = 0; i < world->totalChunks; i++)
    {
        Chunk* chunk = world->chunks[i];

        if (chunk->modified)
            SaveChunk(chunk);
    }
}

static void DestroyChunk(World* world, Chunk* chunk)
{
    if (chunk->modified)
        SaveChunk(chunk);

    DestroyChunkMeshes(chunk);
	AddChunkToPool(world, chunk);
}

// Builds mesh data for the chunk.
static void BuildChunk(World* world, Chunk* chunk)
{
	for (int y = 0; y < CHUNK_SIZE; y++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				Block block = GetBlock(chunk, x, y, z);

				if (block != BLOCK_AIR)
                {
                    BlockMeshType type = g_blockData[block].meshType;
                    Mesh* mesh = chunk->meshes[type];

                    if (mesh == nullptr)
                    {
                        mesh = CreateMesh();
                        chunk->meshes[type] = mesh;
                    }

                    g_blockData[block].buildFunc(chunk, mesh, x, y, z, block);
                }
			}
		}
	}
	
    chunk->state = CHUNK_NEEDS_FILL;
}

static inline void BuildChunkNow(World* world, Chunk* chunk)
{
    BuildChunk(world, chunk);
    FillMeshData(chunk->meshes);
    chunk->state = CHUNK_BUILT;
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
		world->chunks[i] = nullptr;
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

				if (chunk == nullptr)
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

        if (chunk != nullptr && !chunk->active)
            EnqueueChunk(world->destroyQueue, chunk);

        world->chunkHash[c] = nullptr;
	}
}

static void CheckWorld(World* world, Player* player)
{
    Rectf bounds = world->pBounds;
    vec3 pos = player->pos;
    bool shift = false;

    assert(player->pos.x >= 0.0f);
    assert(player->pos.y >= 0.0f);
    assert(player->pos.z >= 0.0f);

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
        CameraFollow(player);
        ShiftWorld(world);
    }
}

static void TryBuildMeshes(World* world, Renderer* rend)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
        rend->meshLists[i].clear();

    for (int i = 0; i < world->visibleCount; i++)
    {
        Chunk* chunk = world->visibleChunks[i];

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
                FillMeshData(chunk->meshes);
                chunk->state = CHUNK_BUILT;
                break;
            }

            case CHUNK_UPDATE:
            {
                DestroyChunkMeshes(chunk);
                BuildChunkNow(world, chunk);
            }

            case CHUNK_BUILT:
            {
                for (int m = 0; m < CHUNK_MESH_COUNT; m++)
                {
                    Mesh* mesh = chunk->meshes[m];

                    if (mesh != nullptr && mesh->vertCount > 0)
                    {
                        mesh->lwPos = (vec3)chunk->lwPos;
                        rend->meshLists[m].push_back(mesh);
                    }
                }

                break;
            }

            default:
                break;
        }
    }
}

#define FRUSTUM_CULLING 1

static void GetVisibleChunks(World* world, Camera* cam)
{
    BEGIN_TIMED_BLOCK(GET_VISIBLE_CHUNKS);
    
    for (int i = 0; i < world->totalChunks; i++)
    {
        Chunk* chunk = world->chunks[i];
        LChunkPos cP = chunk->lcPos;
        vec3 min = cP * CHUNK_SIZE;
        vec3 max = min + (CHUNK_SIZE - 1.0f);

        #if FRUSTUM_CULLING 

        FrustumVisibility visibility = TestFrustum(cam, min, max);

        if (visibility >= FRUSTUM_VISIBLE)
            world->visibleChunks[world->visibleCount++] = chunk;

        #else

        world->visibleChunks[world->visibleCount++] = chunk;

        #endif
    }

    END_TIMED_BLOCK(GET_VISIBLE_CHUNKS);
}

static void UpdateWorld(World* world, Renderer* rend, Player* player)
{
    if (!player->spawned) return;
    
    CheckWorld(world, player);

    world->visibleCount = 0;
    GetCameraPlanes(rend->camera);
    GetVisibleChunks(world, rend->camera);

    TryBuildMeshes(world, rend);

    while (world->destroyQueue.count > 0)
    {
        Chunk* chunk = DequeueChunk(world->destroyQueue);

        if (chunk->state != CHUNK_GENERATING && chunk->state != CHUNK_BUILDING)
            DestroyChunk(world, chunk);
        else EnqueueChunk(world->destroyQueue, chunk);
    }
}

// Returns true if the current block should draw its face when placed
// next to the adjacent block.
static inline bool CanDrawFace(CullType cur, Block adjBlock)
{
    CullType adj = g_blockData[adjBlock].cull;

    if (cur == CULL_OPAQUE)
        return adj >= CULL_TRANSPARENT;
    else return adj == CULL_ALL;
}

#define BLOCK_TRANSPARENT(pos) g_blockData[GetBlock(chunk, pos)].cull >= CULL_TRANSPARENT

static inline Color VertexLight(Chunk* chunk, Axis axis, RelPos pos, int dx, int dy, int dz)
{
    RelPos a, b, c, d;

    switch (axis)
    {
        case AXIS_X:
            a = pos + ivec3(dx, 0, 0);
            b = pos + ivec3(dx, dy, 0);
            c = pos + ivec3(dx, 0, dz);
            d = pos + ivec3(dx, dy, dz);
            break;

        case AXIS_Y:
            a = pos + ivec3(0, dy, 0);
            b = pos + ivec3(dx, dy, 0);
            c = pos + ivec3(0, dy, dz);
            d = pos + ivec3(dx, dy, dz);
            break;

        default:
            a = pos + ivec3(0, 0, dz);
            b = pos + ivec3(dx, 0, dz);
            c = pos + ivec3(0, dy, dz);
            d = pos + ivec3(dx, dy, dz);
            break;
    }

    bool t1 = BLOCK_TRANSPARENT(b);
    bool t2 = BLOCK_TRANSPARENT(c);

    if (t1 || t2) 
    {
        Color c1 = BLOCK_TRANSPARENT(a) ? vec4(1.0f) : vec4(0.25f);
        Color c2 = BLOCK_TRANSPARENT(b) ? vec4(1.0f) : vec4(0.25f);
        Color c3 = BLOCK_TRANSPARENT(c) ? vec4(1.0f) : vec4(0.25f);
        Color c4 = BLOCK_TRANSPARENT(d) ? vec4(1.0f) : vec4(0.25f);

        return Average(c1, c2, c3, c4);
    }
    else 
    {
        Color c1 = GetBlock(chunk, a) == BLOCK_AIR ? vec4(1.0f) : vec4(0.25f);
        Color c2 = GetBlock(chunk, b) == BLOCK_AIR ? vec4(1.0f) : vec4(0.25f);
        Color c3 = GetBlock(chunk, c) == BLOCK_AIR ? vec4(1.0f) : vec4(0.25f);

        return Average(c1, c2, c3);
    }
}

#define VERTEX_LIGHT(a, o1, o2, o3) cull >= CULL_TRANSPARENT ? vec4(1.0f) : VertexLight(chunk, AXIS_##a, rP, o1, o2, o3)

// Builds mesh data for a single block. x, y, and z are relative to the
// chunk in local world space.
static void BuildBlock(Chunk* chunk, Mesh* mesh, int xi, int yi, int zi, Block block)
{
    float* textures = g_blockData[block].textures;

    RelPos rP = ivec3(xi, yi, zi);
    float x = (float)xi, y = (float)yi, z = (float)zi;

    CullType cull = g_blockData[block].cull;

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi + 1, zi)))
    {
        float tex = textures[FACE_TOP];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Y, 1, 1, -1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Y, 1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Y, -1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Y, -1, 1, -1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi - 1, zi)))
    {
        float tex = textures[FACE_BOTTOM];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Y, -1, -1, -1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Y, -1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Y, 1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Y, 1, -1, -1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi, zi + 1)))
    {
        float tex = textures[FACE_FRONT];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Z, -1, -1, 1)); 
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Z, -1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Z, 1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Z, 1, -1, 1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi, yi, zi - 1)))
    {
        float tex = textures[FACE_BACK];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(Z, 1, -1, -1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(Z, 1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(Z, -1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(Z, -1, -1, -1)); 
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi + 1, yi, zi)))
    {
        float tex = textures[FACE_RIGHT];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z + 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(X, 1, -1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z + 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(X, 1, 1, 1));
        SetMeshVertex(mesh, x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(X, 1, 1, -1));
        SetMeshVertex(mesh, x + 0.5f, y - 0.5f, z - 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(X, 1, -1, -1));
    }

    if (CanDrawFace(cull, GetBlock(chunk, xi - 1, yi, zi)))
    {
        float tex = textures[FACE_LEFT];
        SetMeshIndices(mesh);
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z - 0.5f, 0.0f, 1.0f, tex, VERTEX_LIGHT(X, -1, -1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z - 0.5f, 0.0f, 0.0f, tex, VERTEX_LIGHT(X, -1, 1, -1));
        SetMeshVertex(mesh, x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, 0.0f, tex, VERTEX_LIGHT(X, -1, 1, 1));
        SetMeshVertex(mesh, x - 0.5f, y - 0.5f, z + 0.5f, 1.0f, 1.0f, tex, VERTEX_LIGHT(X, -1, -1, 1));
    }
}

static inline void SetBlockTextures(BlockData& data, float top, float bottom, float front, float back, 
    float right, float left)
{
    data.textures[FACE_TOP] = top;
    data.textures[FACE_BOTTOM] = bottom;
    data.textures[FACE_FRONT] = front;
    data.textures[FACE_BACK] = back;
    data.textures[FACE_RIGHT] = right;
    data.textures[FACE_LEFT] = left;
}

static void CreateBlockData()
{
    BlockData& air = g_blockData[BLOCK_AIR];
    air.cull = CULL_ALL;

    BlockData& grass = g_blockData[BLOCK_GRASS];
    SetBlockTextures(grass, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    grass.buildFunc = BuildBlock;
    
    BlockData& dirt = g_blockData[BLOCK_DIRT];
    SetBlockTextures(dirt, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f);
    dirt.buildFunc = BuildBlock;

    BlockData& stone = g_blockData[BLOCK_STONE];
    SetBlockTextures(stone, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f);
    stone.buildFunc = BuildBlock;

    BlockData& water = g_blockData[BLOCK_WATER];
    SetBlockTextures(water, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f);
    water.meshType = MESH_TYPE_FLUID;
    water.buildFunc = BuildBlock;
    water.cull = CULL_TRANSPARENT;

    BlockData& sand = g_blockData[BLOCK_SAND];
    SetBlockTextures(sand, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    sand.buildFunc = BuildBlock;

    BlockData& crate = g_blockData[BLOCK_CRATE];
    SetBlockTextures(crate, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f);
    crate.buildFunc = BuildBlock;

    BlockData& stoneBrick = g_blockData[BLOCK_STONEBRICK];
    SetBlockTextures(stoneBrick, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f);
    stoneBrick.buildFunc = BuildBlock;
}

static World* NewWorld(int loadRangeH, int loadRangeV)
{
    World* world = Calloc<World>();

    // Load range worth of chunks on each side plus the middle chunk.
    world->sizeH = (loadRangeH * 2) + 1;
    world->sizeV = (loadRangeV * 2) + 1;

    world->spawnChunk = ivec3(0, 1, 0);

    world->totalChunks = Square(world->sizeH) * world->sizeV;
    world->chunks = Calloc<Chunk*>(world->totalChunks);
    world->visibleChunks = Calloc<Chunk*>(world->totalChunks);

    ivec3 ref;
    ref.x = world->spawnChunk.x - loadRangeH;
    ref.y = world->spawnChunk.y - loadRangeV;
    ref.z = world->spawnChunk.z - loadRangeH;
    world->ref = ref;

    // Allocate extra chunks for the pool for world shifting. We create new chunks
    // before we destroy the old ones.
    int targetPoolSize = world->totalChunks * 2;
    world->pool = Calloc<Chunk*>(targetPoolSize);
    world->poolSize = 0;
    world->maxPoolSize = targetPoolSize;

    for (int i = 0; i < targetPoolSize; i++)
    {
        Chunk* chunk = Malloc<Chunk>();
        AddChunkToPool(world, chunk);
    }

    float minH = (float)(loadRangeH * CHUNK_SIZE);
    float maxH = minH + CHUNK_SIZE;

    float minV = (float)(loadRangeV * CHUNK_SIZE);
    float maxV = minV + CHUNK_SIZE;

    world->pBounds = NewRect(vec3(minH, minV, minH), vec3(maxH, maxV, maxH));

    CreateBlockData();

    char path[MAX_PATH];
    sprintf(path, "%s\\WorldData.txt", PathToExe("Saves"));

    if (!PathFileExists(path))
    {
        srand((uint32_t)time(0));
        world->seed = rand();
    }
    else
    {
        int seed;
        ReadBinary(path, (char*)&seed);
        world->seed = seed;
    }

    ShiftWorld(world);

    return world;
}
