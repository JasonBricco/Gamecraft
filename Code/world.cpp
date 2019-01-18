//
// Jason Bricco
//

#if _DEBUG

static bool ChunkIsValid(World* world, Chunk* chunk)
{
    if (chunk == nullptr) return true;

    LChunkPos lP = chunk->lcPos;

    int lim = world->size;

    if (lP.x < 0 || lP.y < 0 || lP.x >= lim || lP.y >= lim)
        return false;

    if (chunk->state < 0 || chunk->state >= CHUNK_STATE_COUNT)
        return false;

    for (int i = 0; i < CHUNK_SIZE_3; i++)
    {
        Block block = chunk->blocks[i];

        if (block < 0 || block >= BLOCK_COUNT)
            return false;
    }

    return true;
}

static bool RegionIsValid(Region region)
{
    for (int i = 0; i < REGION_SIZE_3; i++)
    {
        SerializedChunk* chunk = region.chunks + i;

        if (chunk->size > chunk->maxSize)
            return false;

        if (chunk->size < 0 || chunk->maxSize < 0 || chunk->size > 40000 || chunk->maxSize > 40000)
            return false;
    }

    return true;
}

#else

#define ChunkIsValid(world, chunk)
#define RegionIsValid(region)

#endif

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

static inline bool BlockInsideChunk(int x, int y, int z)
{
    return x >= 0 && x < CHUNK_SIZE_X && y >= 0 && y < CHUNK_SIZE_Y && z >= 0 && z < CHUNK_SIZE_X;
}

static inline bool BlockInsideChunk(RelPos p)
{
    return BlockInsideChunk(p.x, p.y, p.z);
}

static inline bool BlockInsideWorld(World* world, int x, int z)
{
    int wSize = world->size * CHUNK_SIZE_X;
    return x >= 0 && z >= 0 && x < wSize && z < wSize;
}

static inline bool ChunkInsideWorld(World* world, int x, int z)
{
    return x >= 0 && x < world->size && z >= 0 && z < world->size;
}

static inline RelPos LWorldToRelPos(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX & CHUNK_MASK, lwY, lwZ & CHUNK_MASK);
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
    assert(ChunkInsideWorld(world, lcX, lcZ));
	return world->chunks[ChunkIndex(world, lcX, lcZ)];
}

static inline Chunk* GetChunk(World* world, LChunkPos pos)
{
	return GetChunk(world, pos.x, pos.z);
}

static inline Chunk* GetChunkSafe(World* world, LChunkPos pos)
{
    if (!ChunkInsideWorld(world, pos.x, pos.z))
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

static inline RebasedPos Rebase(World* world, LChunkPos lP, int rX, int rZ)
{
    assert(ChunkInsideWorld(world, lP.x, lP.z));
    
    if (rX < 0)
        return Rebase(world, lP + DIRECTIONS_2D[DIRECTION_LEFT], CHUNK_SIZE_X + rX, rZ);

    if (rX >= CHUNK_SIZE_X) 
        return Rebase(world, lP + DIRECTIONS_2D[DIRECTION_RIGHT], rX - CHUNK_SIZE_X, rZ);

    if (rZ < 0)
        return Rebase(world, lP + DIRECTIONS_2D[DIRECTION_BACK], rX, CHUNK_SIZE_X + rZ);
    
    if (rZ >= CHUNK_SIZE_X)
        return Rebase(world, lP + DIRECTIONS_2D[DIRECTION_FRONT], rX, rZ - CHUNK_SIZE_X);

    return { GetChunk(world, lP), rX, rZ };
}

static inline Block GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
    assert(rY >= 0 && rY < CHUNK_SIZE_Y);
    int index = rX + CHUNK_SIZE_X * (rY + CHUNK_SIZE_Y * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    Block block = chunk->blocks[index];
    assert(block >= 0 && block < BLOCK_COUNT);
    return block;
}

static inline Block GetBlock(Chunk* chunk, RelPos pos)
{
    return GetBlock(chunk, pos.x, pos.y, pos.z);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, int rX, int rY, int rZ)
{
    assert(chunk->state >= CHUNK_LOADED);

    if (rY < 0) return BLOCK_STONE;
    if (rY >= WORLD_HEIGHT) return BLOCK_AIR;

    RebasedPos p = Rebase(world, chunk->lcPos, rX, rZ);
    return GetBlock(p.chunk, p.rX, rY, p.rZ);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, RelPos p)
{
    return GetBlockSafe(world, chunk, p.x, p.y, p.z);
}

static Block GetBlock(World* world, int lwX, int lwY, int lwZ)
{
    if (lwY < 0) return BLOCK_STONE;
    if (lwY >= WORLD_HEIGHT) return BLOCK_AIR;

    assert(BlockInsideWorld(world, lwX, lwZ));

    LChunkPos lcPos = LWorldToLChunkPos(lwX, lwZ);
    Chunk* chunk = GetChunk(world, lcPos);

    if (chunk == nullptr || chunk->state < CHUNK_LOADED)
        return BLOCK_STONE;

    RelPos rPos = LWorldToRelPos(lwX, lwY, lwZ);
    return GetBlock(chunk, rPos);
}

static inline Block GetBlock(World* world, LWorldPos pos)
{
    return GetBlock(world, pos.x, pos.y, pos.z);
}

static inline void SetBlock(Chunk* chunk, int rX, int rY, int rZ, Block block)
{
    assert(block >= 0 && block < BLOCK_COUNT);
    int index = rX + CHUNK_SIZE_X * (rY + CHUNK_SIZE_Y * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

static inline void SetBlock(Chunk* chunk, RelPos pos, Block block)
{
    SetBlock(chunk, pos.x, pos.y, pos.z, block);
}

static void FlagChunkForUpdate(World* world, Chunk* chunk, LChunkPos lP, RelPos rP)
{
    chunk->pendingUpdate = true;
    chunk->modified = true;

    for (int i = 0; i < 8; i++)
    {
        RelPos nextRel = rP + DIRECTIONS_2D[i];
        Chunk* adj = Rebase(world, lP, nextRel.x, nextRel.z).chunk;
        assert(adj->lcPos.x > 0 && adj->lcPos.z > 0 && adj->lcPos.x < world->size - 1 && adj->lcPos.z < world->size - 1);
        adj->pendingUpdate = true;
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

    if (GetBlock(chunk, rP.x, rP.y, rP.z) != block)
    {
        SetBlock(chunk, rP.x, rP.y, rP.z, block);
        PlaySound(GetBlockSetSound(world, block));
        FlagChunkForUpdate(world, chunk, lP, rP);
    }
}

static inline void SetBlock(World* world, int lwX, int lwY, int lwZ, Block block)
{
	SetBlock(world, ivec3(lwX, lwY, lwZ), block);
}

static void FillChunk(Chunk* chunk, Block block)
{
    for (int i = 0; i < CHUNK_SIZE_3; i++)
        chunk->blocks[i] = block;
}

// Fill a chunk with a single block type.
static void FillChunk(World* world, Chunk* chunk, Block block)
{
    FillChunk(chunk, block);

    // Rebuild meshes for this chunk and all neighbor chunks.
    chunk->pendingUpdate = true;

    for (int i = 0; i < 8; i++)
        GetChunk(world, chunk->lcPos + DIRECTIONS_2D[i])->pendingUpdate = true;
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
        return new Chunk();

	Chunk* chunk = world->pool[world->poolSize - 1];
	world->poolSize--;
	return chunk;
}

static void LoadChunk(World* world, Chunk* chunk)
{
    if (!LoadChunkFromDisk(world, chunk))
        GenerateChunkTerrain(world, chunk);

    assert(ChunkIsValid(world, chunk));
    chunk->state = CHUNK_LOADED;
}

static Chunk* CreateChunk(GameState* state, World* world, int lcX, int lcZ, ChunkPos cPos)
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
        QueueAsync(state, LoadChunk, world, chunk);
		world->chunks[index] = chunk;
	}

	return chunk;
}

static void DestroyChunkCallback(World* world, Chunk* chunk)
{
    for (int i = 0; i < CHUNK_MESH_COUNT; i++)
       DestroyMesh(chunk->meshes[i]);

    AddChunkToPool(world, chunk);
}

static void DestroyChunk(GameState* state, World* world, Chunk* chunk)
{
    if (chunk->modified)
        QueueAsync(state, SaveChunk, world, chunk, DestroyChunkCallback);
    else DestroyChunkCallback(world, chunk);
}

// To allow "infinite" terrain, the world is always located near the origin.
// This function fills the world near the origin based on the reference
// world position within the world.
static void ShiftWorld(GameState* state, World* world)
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
        CreateChunk(state, world, p.x, p.y, ivec3(p.z, 0, p.w));
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

static void CheckWorld(GameState* state, World* world, Player* player)
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
        MoveCamera(state->camera, player->pos);
        ShiftWorld(state, world);
    }
}

static void UpdateWorld(GameState* state, World* world, Camera* cam, Player* player)
{
    world->visibleCount = 0;

    if (!player->spawned)
    {
        Chunk* spawnChunk = GetChunk(world, ChunkToLChunkPos(world->spawnChunk, world->ref));

        if (spawnChunk->state >= CHUNK_LOADED)
            SpawnPlayer(state, player, world->pBounds);
    }
    else
    {
        world->playerRegion = LWorldToRegionPos(player->pos, world->ref);

        if (world->buildCount == 0)
            CheckWorld(state, world, player);

        GetCameraPlanes(cam);
        GetVisibleChunks(world, cam);

        ProcessVisibleChunks(state, world, cam);

        while (world->destroyQueue.count > 0)
        {
            Chunk* chunk = DequeueChunk(world->destroyQueue);

            if (chunk->state != CHUNK_LOADING)
                DestroyChunk(state, world, chunk);
            else EnqueueChunk(world->destroyQueue, chunk);
        }
    }
}

static World* NewWorld(GameState* state, int loadRange, WorldConfig& config, World* existing = nullptr)
{
    World* world;

    if (existing == nullptr)
    {
        world = PushStruct(World);
        Construct(world, World);

        // Load range worth of chunks on each side plus the middle chunk.
        world->size = (loadRange * 2) + 1;

        world->totalChunks = Square(world->size);
        world->chunks = (Chunk**)calloc(world->totalChunks, sizeof(Chunk*));
        world->visibleChunks = (Chunk**)calloc(world->totalChunks, sizeof(Chunk*));

        world->chunksToCreate.reserve(world->totalChunks);
        world->loadRange = loadRange;

        // Allocate extra chunks for the pool for world shifting. We create new chunks
        // before we destroy the old ones.
        int targetPoolSize = world->totalChunks * 2;
        world->pool = (Chunk**)calloc(targetPoolSize, sizeof(Chunk*));
        world->poolSize = 0;
        world->maxPoolSize = targetPoolSize;

        for (int i = 0; i < targetPoolSize; i++)
        {
            Chunk* chunk = new Chunk();
            AddChunkToPool(world, chunk);
        }

        float min = (float)(loadRange * CHUNK_SIZE_X);
        float max = min + CHUNK_SIZE_X;

        world->pBounds = NewRect(vec3(min, 0.0f, min), vec3(max, 0.0f, max));

        CreateBlockData(state, world->blockData);

        world->savePath = PathToExe("Saves", PushArray(MAX_PATH, char), MAX_PATH);
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

        for (int i = 0; i < MESH_POOL_CAPACITY; i++)
            AllocMeshData(&world->meshData[i], 131072, 32768);

        world->meshDataCount = MESH_POOL_CAPACITY;
        world->blockToSet = BLOCK_GRASS;

        world->regionMutex = CreateMutex(NULL, FALSE, NULL);
    }
    else 
    {
        world = existing;

        // The callback method skips saving the chunk if it is modified. 
        // We don't want to save any chunks here.
        for (int i = 0; i < world->totalChunks; i++)
        {
            DestroyChunkCallback(world, world->chunks[i]);
            world->chunks[i] = nullptr;
        }

        world->seed = rand();
    }

    world->radius = config.infinite ? INT_MAX : config.radius;
    world->falloffRadius = world->radius - (CHUNK_SIZE_X * 2);

    world->spawnChunk = ivec3(0);

    ivec3 ref;
    ref.x = world->spawnChunk.x - loadRange;
    ref.z = world->spawnChunk.z - loadRange;
    world->ref = ref;

    ShiftWorld(state, world);

    return world;
}

static void RegenerateWorld(GameState* state, World* world, WorldConfig& config)
{
    DeleteDirectory(world->savePath);
    DeleteRegions(world);
    world = NewWorld(state, world->loadRange, config, world);
}
