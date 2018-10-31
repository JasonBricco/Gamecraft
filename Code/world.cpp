//
// Jason Bricco
//

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

static inline RelPos LWorldToRelPos(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX & CHUNK_SIZE_X - 1, lwY, lwZ & CHUNK_SIZE_X - 1);
}

static inline RelPos LWorldToRelPos(LWorldPos wPos)
{
	return LWorldToRelPos(wPos.x, wPos.y, wPos.z);
}

static inline LChunkPos LWorldToLChunkPos(int lwX, int lwZ)
{
	return ivec3(lwX >> CHUNK_SIZE_BITS, 0, lwZ >> CHUNK_SIZE_BITS);
}

static inline LChunkPos LWorldToLChunkPos(LWorldPos pos)
{
	return LWorldToLChunkPos(pos.x, pos.z);
}

static inline LChunkPos LWorldToLChunkPos(vec3 wPos)
{
	return LWorldToLChunkPos((int)wPos.x, (int)wPos.z);
}

static inline LChunkPos ChunkToLChunkPos(ChunkPos pos, ChunkPos ref)
{
    return pos - ref;
}

static inline ChunkPos LChunkToChunkPos(LChunkPos pos, ChunkPos ref)
{
    return ref + pos;
}

static inline RegionPos ChunkToRegionPos(ChunkPos pos)
{
    vec3 tmp = vec3(pos.x / (float)REGION_SIZE, 0.0f, pos.z / (float)REGION_SIZE);
    return ivec3(FloorToInt(tmp.x), 0, FloorToInt(tmp.z));
}

static inline RegionPos LWorldToRegionPos(vec3 wPos, ChunkPos ref)
{
    LChunkPos lP = LWorldToLChunkPos(wPos);
    ChunkPos cP = LChunkToChunkPos(lP, ref);
    return ChunkToRegionPos(cP);
}

// Returns an index into the chunks array from the given chunk position.
static inline int32_t ChunkIndex(World* world, int32_t lcX, int32_t lcZ)
{
    return lcZ * world->size + lcX;
}

static inline Chunk* GetChunk(World* world, int32_t lcX, int32_t lcZ)
{
	return world->chunks[ChunkIndex(world, lcX, lcZ)];
}

static inline Chunk* GetChunk(World* world, LChunkPos pos)
{
	return GetChunk(world, pos.x, pos.z);
}

static inline Chunk* GetChunkSafe(World* world, LChunkPos pos)
{
    if (pos.x < 0 || pos.x >= world->size || pos.z < 0 || pos.z >= world->size)
        return nullptr;

    return GetChunk(world, pos);
}

static inline uint32_t ChunkHashBucket(int32_t x, int32_t z)
{
	uint32_t hashValue = (31 + x) * 23 + z;
	return hashValue & (CHUNK_HASH_SIZE - 1);
}

static inline uint32_t ChunkHashBucket(ChunkPos p)
{
    return ChunkHashBucket(p.x, p.z);
}

// The chunk hash stores existing chunks during a world shift. Chunks will be
// pulled out from the table to fill the new world section if applicable.
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

static inline Chunk* ChunkFromHash(World* world, ChunkPos p)
{
	return ChunkFromHash(world, ChunkHashBucket(p.x, p.z), p);
}

static inline Chunk* ChunkFromHash(World* world, int32_t x, int32_t y)
{
    return ChunkFromHash(world, ivec3(x, 0, y));
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

static inline void SetBlock(Chunk* chunk, int rX, int rY, int rZ, Block block)
{
    int index = rX + CHUNK_SIZE_X * (rY + CHUNK_SIZE_Y * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

static inline void SetBlock(Chunk* chunk, RelPos pos, Block block)
{
    SetBlock(chunk, pos.x, pos.y, pos.z, block);
}

static inline Chunk* GetRelChunk(World* world, LChunkPos lP, int rX, int rZ)
{
    if (rX < 0)
        return GetRelChunk(world, lP + DIRECTIONS[LEFT], CHUNK_SIZE_X + rX, rZ);

    if (rX >= CHUNK_SIZE_X) 
        return GetRelChunk(world, lP + DIRECTIONS[RIGHT], rX - CHUNK_SIZE_X, rZ);

    if (rZ < 0)
        return GetRelChunk(world, lP + DIRECTIONS[BACK], rX, CHUNK_SIZE_X + rZ);
    
    if (rZ >= CHUNK_SIZE_X)
        return GetRelChunk(world, lP + DIRECTIONS[FRONT], rX, rZ - CHUNK_SIZE_X);

    return GetChunk(world, lP);
}

static void FlagChunkForUpdate(World* world, Chunk* chunk, LChunkPos lP, RelPos rP)
{
    chunk->state = CHUNK_UPDATE;
    chunk->modified = true;

    for (int i = 0; i < 8; i++)
    {
        RelPos nextRel = rP + DIRECTIONS[i];
        Chunk* adj = GetRelChunk(world, lP, nextRel.x, nextRel.z);
        assert(adj->lcPos.x > 0 && adj->lcPos.z > 0 && adj->lcPos.x < world->size - 1 && adj->lcPos.z < world->size - 1);
        adj->state = CHUNK_UPDATE;
    }
}

static inline void SetBlock(World* world, LWorldPos wPos, Block block)
{
    if (wPos.y < 0 || wPos.y >= WORLD_HEIGHT) return;

    if (OverlapsBlock(world->player, wPos.x, wPos.y, wPos.z))
        return;
    
	LChunkPos lP = LWorldToLChunkPos(wPos);

    // We cannot set a block to an edge chunk - the edges are buffer chunks and should
    // not have their meshes built.
    assert(lP.x > 0 && lP.z > 0 && lP.x < world->size - 1 && lP.z < world->size - 1);

	Chunk* chunk = GetChunk(world, lP);
	assert(chunk != nullptr);

	RelPos rP = LWorldToRelPos(wPos);
	SetBlock(chunk, rP.x, rP.y, rP.z, block);

    FlagChunkForUpdate(world, chunk, lP, rP);
}

static inline void SetBlock(World* world, int lwX, int lwY, int lwZ, Block block)
{
	SetBlock(world, ivec3(lwX, lwY, lwZ), block);
}

static inline Block GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
    if (rY < 0) return BLOCK_STONE;
    if (rY >= WORLD_HEIGHT) return BLOCK_AIR;

    int index = rX + CHUNK_SIZE_X * (rY + CHUNK_SIZE_Y * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    return chunk->blocks[index];
}

static inline Block GetBlock(Chunk* chunk, RelPos pos)
{
    return GetBlock(chunk, pos.x, pos.y, pos.z);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, int rX, int rY, int rZ)
{
    if (rY < 0) return BLOCK_STONE;
    if (rY >= WORLD_HEIGHT) return BLOCK_AIR;

    if (rX < 0)
        return GetBlockSafe(world, GetChunk(world, chunk->lcPos + DIRECTIONS[LEFT]), CHUNK_SIZE_X + rX, rY, rZ);

    if (rX >= CHUNK_SIZE_X) 
        return GetBlockSafe(world, GetChunk(world, chunk->lcPos + DIRECTIONS[RIGHT]), rX - CHUNK_SIZE_X, rY, rZ);

    if (rZ < 0) 
        return GetBlockSafe(world, GetChunk(world, chunk->lcPos + DIRECTIONS[BACK]), rX, rY, CHUNK_SIZE_X + rZ);
    
    if (rZ >= CHUNK_SIZE_X) 
        return GetBlockSafe(world, GetChunk(world, chunk->lcPos + DIRECTIONS[FRONT]), rX, rY, rZ - CHUNK_SIZE_X);

    return GetBlock(chunk, rX, rY, rZ);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, RelPos p)
{
    return GetBlockSafe(world, chunk, p.x, p.y, p.z);
}

static Block GetBlock(World* world, int lwX, int lwY, int lwZ)
{
	LChunkPos lcPos = LWorldToLChunkPos(lwX, lwZ);
	Chunk* chunk = GetChunk(world, lcPos);
	assert(chunk != nullptr);

	RelPos rPos = LWorldToRelPos(lwX, lwY, lwZ);
	return GetBlock(chunk, rPos);
}

static inline Block GetBlock(World* world, LWorldPos pos)
{
	return GetBlock(world, pos.x, pos.y, pos.z);
}

// Fill a chunk with a single block type.
static void FillChunk(Chunk* chunk, Block block)
{
    for (int z = 0; z < CHUNK_SIZE_X; z++)
    {
        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int x = 0; x < CHUNK_SIZE_X; x++)
                SetBlock(chunk, x, y, z, block);
        }
    }
}

static inline void AddChunkToPool(World* world, Chunk* chunk)
{
    if (world->poolSize + 1 > world->maxPoolSize)
    {
        int newMax = world->maxPoolSize + (world->totalChunks / 2);
        world->pool = Realloc<Chunk*>(world->pool, newMax);
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

static Chunk* CreateChunk(World* world, int lcX, int lcZ, ChunkPos cPos)
{
    int index = ChunkIndex(world, lcX, lcZ);
	Chunk* chunk = world->chunks[index];

	if (chunk == nullptr)
	{
		chunk = ChunkFromPool(world);
		chunk->lcPos = ivec3(lcX, 0, lcZ);
		chunk->cPos = cPos;
        chunk->lwPos = chunk->lcPos * CHUNK_SIZE_X;
        chunk->active = true;
        QueueAsync(LoadChunk, world, chunk);
		world->chunks[index] = chunk;
	}

	return chunk;
}

static void DestroyChunk(World* world, Chunk* chunk)
{
    if (chunk->modified)
        SaveChunk(world, chunk);

    DestroyChunkMeshes(chunk);
	AddChunkToPool(world, chunk);
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
	for (int z = 0; z < world->size; z++)
	{
		for (int x = 0; x < world->size; x++)
		{
			int wX = world->ref.x + x;
			int wZ = world->ref.z + z;

			Chunk* chunk = ChunkFromHash(world, wX, wZ);

			if (chunk == nullptr)
                world->chunksToCreate.push_back(ivec4(x, z, wX, wZ));
			else 
			{
				chunk->lcPos = ivec3(x, 0, z);
                chunk->lwPos = chunk->lcPos * CHUNK_SIZE_X;
				world->chunks[ChunkIndex(world, x, z)] = chunk;
			}
		}
	}

    float pCoord = (float)(world->loadRange * CHUNK_SIZE_X);
    vec2 playerChunk = vec2(pCoord, pCoord);

    sort(world->chunksToCreate.begin(), world->chunksToCreate.end(), [playerChunk](auto a, auto b) 
    { 
        float distA = distance2(vec2(a.x, a.y), playerChunk);
        float distB = distance2(vec2(b.x, b.y), playerChunk);
        return distA < distB;
    });

    for (int i = 0; i < world->chunksToCreate.size(); i++)
    {
        // Encoded ivec4 values as x, y = local x, z and z, w = world x, z.
        ivec4 p = world->chunksToCreate[i];
        CreateChunk(world, p.x, p.y, ivec3(p.z, 0, p.w));
    }

    world->chunksToCreate.clear();

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
        MoveCamera(player->camera, player->pos);
        ShiftWorld(world);
    }
}

static void UpdateWorld(World* world, Renderer* rend, Player* player)
{
    if (!player->spawned)
    {
        Chunk* spawnChunk = GetChunk(world, ChunkToLChunkPos(world->spawnChunk, world->ref));

        if (spawnChunk->state >= CHUNK_LOADED)
            SpawnPlayer(player, world->pBounds);
    }
    else
    {
        world->playerRegion = LWorldToRegionPos(player->pos, world->ref);

        if (world->chunksBuilding == 0)
            CheckWorld(world, player);

        world->visibleCount = 0;
        GetCameraPlanes(rend->camera);
        GetVisibleChunks(world, rend->camera);

        TryBuildMeshes(world, rend);

        while (world->destroyQueue.count > 0)
        {
            Chunk* chunk = DequeueChunk(world->destroyQueue);

            if (chunk->state != CHUNK_LOADING)
                DestroyChunk(world, chunk);
            else EnqueueChunk(world->destroyQueue, chunk);
        }
    }
}

static World* NewWorld(int loadRange)
{
    World* world = Calloc<World>();

    // Load range worth of chunks on each side plus the middle chunk.
    world->size = (loadRange * 2) + 1;

    world->spawnChunk = ivec3(0);

    world->totalChunks = Square(world->size);
    world->chunks = Calloc<Chunk*>(world->totalChunks);
    world->visibleChunks = Calloc<Chunk*>(world->totalChunks);

    world->chunksToCreate.reserve(world->totalChunks);

    world->loadRange = loadRange;

    ivec3 ref;
    ref.x = world->spawnChunk.x - loadRange;
    ref.z = world->spawnChunk.z - loadRange;
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

    float min = (float)(loadRange * CHUNK_SIZE_X);
    float max = min + CHUNK_SIZE_X;

    world->pBounds = NewRect(vec3(min, 0.0f, min), vec3(max, 0.0f, max));

    CreateBlockData();

    world->savePath = PathToExe("Saves");
    CreateDirectory(world->savePath, NULL);

    char path[MAX_PATH];
    sprintf(path, "%s\\WorldData.txt", world->savePath);

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
