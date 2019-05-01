//
// Gamecraft
//

static inline bool BlockInsideChunk(int x, int y, int z)
{
    return x >= 0 && x < CHUNK_SIZE_H && y >= 0 && y < CHUNK_SIZE_V && z >= 0 && z < CHUNK_SIZE_H;
}

static inline bool BlockInsideChunk(RelP p)
{
    return BlockInsideChunk(p.x, p.y, p.z);
}

static inline bool BlockInsideWorldH(World* world, int x, int z)
{
    int wSize = world->size * CHUNK_SIZE_H;
    return x >= 0 && z >= 0 && x < wSize && z < wSize;
}

static inline bool GroupInsideWorld(World* world, int x, int z)
{
    return x >= 0 && x < world->size && z >= 0 && z < world->size;
}

static inline bool ChunkInsideGroup(int y)
{
    return y >= 0 && y < WORLD_CHUNK_HEIGHT;
}

static inline bool ChunkInsideWorld(World* world, int x, int y, int z)
{
    return GroupInsideWorld(world, x, z) && ChunkInsideGroup(y);
}

static inline bool IsEdgeChunk(World* world, Chunk* chunk)
{
    LChunkP p = chunk->lcPos;
    return p.x == 0 || p.z == 0 || p.x == world->size - 1 || p.z == world->size - 1;
}

static inline bool IsEdgeGroup(World* world, ChunkGroup* group)
{
    return IsEdgeChunk(world, group->chunks);
}

static inline RelP LWorldToRelP(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX & CHUNK_H_MASK, lwY & CHUNK_V_MASK, lwZ & CHUNK_H_MASK);
}

static inline RelP LWorldToRelP(LWorldP wPos)
{
	return LWorldToRelP(wPos.x, wPos.y, wPos.z);
}

static inline LChunkP LWorldToLChunkP(int lwX, int lwY, int lwZ)
{
	return ivec3(lwX >> CHUNK_H_BITS, lwY >> CHUNK_V_BITS, lwZ >> CHUNK_H_BITS);
}

static inline LChunkP LWorldToLChunkP(LWorldP pos)
{
	return LWorldToLChunkP(pos.x, pos.y, pos.z);
}

static inline LChunkP LWorldToLChunkP(vec3 wPos)
{
	return LWorldToLChunkP((int)wPos.x, (int)wPos.y, (int)wPos.z);
}

static inline LChunkP ChunkToLChunkP(ChunkP pos, ChunkP ref)
{
    return pos - ref;
}

static inline ChunkP LChunkToChunkP(LChunkP pos, ChunkP ref)
{
    return ref + pos;
}

static inline LWorldP LChunkToLWorldP(LChunkP p)
{
    return ivec3(p.x * CHUNK_SIZE_H, p.y * CHUNK_SIZE_V, p.z * CHUNK_SIZE_H);
}

static inline WorldP ChunkToWorldP(ChunkP p)
{
    return LChunkToLWorldP(p);
}

static inline ChunkP WorldToChunkP(WorldP p)
{
    return LWorldToLChunkP(p);
}

static inline RegionP ChunkToRegionP(ChunkP pos)
{
    // We must use division instead of shifting since region positions can be negative.
    vec3 tmp = vec3(pos.x / (float)REGION_SIZE, 0.0f, pos.z / (float)REGION_SIZE);
    return ivec3(FloorToInt(tmp.x), 0, FloorToInt(tmp.z));
}

static inline RegionP LWorldToRegionP(vec3 wPos, ChunkP ref)
{
    LChunkP lP = LWorldToLChunkP(wPos);
    ChunkP cP = LChunkToChunkP(lP, ref);
    return ChunkToRegionP(cP);
}

static inline WorldP LWorldToWorldP(World* world, LWorldP p)
{
    LChunkP lcP = LWorldToLChunkP(p);
    ChunkP cP = LChunkToChunkP(lcP, world->ref);
    WorldP wP = ChunkToWorldP(cP);
    RelP rel = LWorldToRelP(p);
    return ivec3(wP.x + rel.x, p.y, wP.z + rel.z);
}

// Returns an index into the chunk group array from the given chunk position.
static inline int32_t GroupIndex(World* world, int32_t lcX, int32_t lcZ)
{
    return lcZ * world->size + lcX;
}

static inline ChunkGroup* GetGroup(World* world, int32_t lcX, int32_t lcZ)
{
    assert(GroupInsideWorld(world, lcX, lcZ));
    return world->groups[GroupIndex(world, lcX, lcZ)];
}

static inline ChunkGroup* GetGroupSafe(World* world, int32_t lcX, int32_t lcZ)
{
    if (!GroupInsideWorld(world, lcX, lcZ))
        return nullptr;

    return GetGroup(world, lcX, lcZ);
}

static inline Chunk* GetChunk(World* world, int32_t lcX, int32_t lcY, int32_t lcZ)
{
    assert(ChunkInsideGroup(lcY));
    return GetGroup(world, lcX, lcZ)->chunks + lcY;
}

static inline Chunk* GetChunk(World* world, LChunkP pos)
{
	return GetChunk(world, pos.x, pos.y, pos.z);
}

static inline Chunk* GetChunk(ChunkGroup* group, int y)
{
    return group->chunks + y;
}

static inline Chunk* GetChunkSafe(World* world, LChunkP pos)
{
    if (!ChunkInsideWorld(world, pos.x, pos.y, pos.z))
        return nullptr;

    return GetChunk(world, pos);
}

static inline uint32_t GroupHashBucket(int32_t x, int32_t z)
{
	uint32_t hashValue = x + (31 * z);
	return hashValue & (GROUP_HASH_SIZE - 1);
}

static inline uint32_t GroupHashBucket(ChunkP p)
{
    return GroupHashBucket(p.x, p.z);
}

// The group hash stores existing chunks during a world shift. Groups will be
// pulled out from the table to fill the new world section if applicable.
// Otherwise, new chunks will be created and the ones remaining in the pool
// will be destroyed.
static ChunkGroup* GroupFromHash(World* world, uint32_t bucket, ChunkP pos)
{
    ChunkGroup* group = world->groupHash[bucket];

    if (group == nullptr) 
        return nullptr;

    if (group->pos == pos)
    {
        group->active = true;
        return group;
    }

    uint32_t firstBucket = bucket;

    while (true)
    {
        bucket = (bucket + 1) & (GROUP_HASH_SIZE - 1);

        if (bucket == firstBucket)
            return nullptr;

        group = world->groupHash[bucket];

        if (group != nullptr && group->pos == pos)
        {
            group->active = true;
            return group;
        }
    }
}

static inline ChunkGroup* GroupFromHash(World* world, ChunkP p)
{
	return GroupFromHash(world, GroupHashBucket(p.x, p.z), p);
}

static inline ChunkGroup* GroupFromHash(World* world, int32_t x, int32_t z)
{
    return GroupFromHash(world, ivec3(x, 0, z));
}

static inline void AddGroupToHash(World* world, ChunkGroup* group)
{
	if (group == nullptr) return;

	uint32_t bucket = GroupHashBucket(group->pos);

    #if _DEBUG
    uint32_t firstBucket = bucket;
    #endif

    while (world->groupHash[bucket] != nullptr)
    {
        bucket = (bucket + 1) & (GROUP_HASH_SIZE - 1);
        assert(bucket != firstBucket);
    }

    group->active = false;
    world->groupHash[bucket] = group;
}

static inline RebasedPos Rebase(World* world, LChunkP lP, int rX, int rY, int rZ)
{
    assert(ChunkInsideWorld(world, lP.x, lP.y, lP.z));
    
    if (rX < 0)
        return Rebase(world, lP + DIR_LEFT, CHUNK_SIZE_H + rX, rY, rZ);

    if (rX >= CHUNK_SIZE_H) 
        return Rebase(world, lP + DIR_RIGHT, rX - CHUNK_SIZE_H, rY, rZ);

    if (rZ < 0)
        return Rebase(world, lP + DIR_BACK, rX, rY, CHUNK_SIZE_H + rZ);
    
    if (rZ >= CHUNK_SIZE_H)
        return Rebase(world, lP + DIR_FRONT, rX, rY, rZ - CHUNK_SIZE_H);

    if (rY < 0)
    {
        LChunkP tar = lP + DIR_DOWN;

        if (tar.y >= 0 && tar.y < WORLD_CHUNK_HEIGHT)
            return Rebase(world, lP + DIR_DOWN, rX, CHUNK_SIZE_V + rY, rZ);
        else return { nullptr, 0, -1, 0 };
    }

    if (rY >= CHUNK_SIZE_V)
    {
        LChunkP tar = lP + DIR_UP;
        
        if (tar.y >= 0 && tar.y < WORLD_CHUNK_HEIGHT)
            return Rebase(world, lP + DIR_UP, rX, rY - CHUNK_SIZE_V, rZ);
        else return { nullptr, 0, 1, 0 };
    }

    return { GetChunk(world, lP), rX, rY, rZ };
}

static inline RebasedPos Rebase(World* world, LChunkP lP, RelP rP)
{
    return Rebase(world, lP, rP.x, rP.y, rP.z);
}

static inline Block GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
    assert(rY >= 0 && rY < CHUNK_SIZE_V);
    int index = rX + CHUNK_SIZE_H * (rY + CHUNK_SIZE_V * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    Block block = chunk->blocks[index];
    assert(block >= 0 && block < BLOCK_COUNT);
    return block;
}

static inline Block GetBlock(Chunk* chunk, RelP pos)
{
    return GetBlock(chunk, pos.x, pos.y, pos.z);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, int rX, int rY, int rZ)
{
    assert(GetGroup(world, chunk->lcPos.x, chunk->lcPos.z)->loaded);
    RebasedPos p = Rebase(world, chunk->lcPos, rX, rY, rZ);

    if (p.chunk == nullptr)
    {
        assert(p.rY == -1 || p.rY == 1);
        return p.rY == -1 ? BLOCK_STONE : BLOCK_AIR;
    }

    return GetBlock(p.chunk, p.rX, p.rY, p.rZ);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, RelP p)
{
    return GetBlockSafe(world, chunk, p.x, p.y, p.z);
}

static Block GetBlock(World* world, int lwX, int lwY, int lwZ)
{
    if (lwY < 0) return BLOCK_STONE;

    if (lwY >= WORLD_BLOCK_HEIGHT || !BlockInsideWorldH(world, lwX, lwZ))
        return BLOCK_AIR;

    LChunkP lcPos = LWorldToLChunkP(lwX, lwY, lwZ);
    ChunkGroup* group = GetGroup(world, lcPos.x, lcPos.z);

    if (group == nullptr || !group->loaded)
        return BLOCK_STONE;

    RelP rPos = LWorldToRelP(lwX, lwY, lwZ);
    return GetBlock(GetChunk(group, lcPos.y), rPos);
}

static inline Block GetBlock(World* world, LWorldP pos)
{
    return GetBlock(world, pos.x, pos.y, pos.z);
}

static inline void SetBlock(Chunk* chunk, int index, Block block)
{
    assert(block >= 0 && block < BLOCK_COUNT);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

static inline void SetBlock(Chunk* chunk, int rX, int rY, int rZ, Block block)
{
    assert(block >= 0 && block < BLOCK_COUNT);
    int index = rX + CHUNK_SIZE_H * (rY + CHUNK_SIZE_V * rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

static inline void SetBlock(Chunk* chunk, RelP pos, Block block)
{
    SetBlock(chunk, pos.x, pos.y, pos.z, block);
}

static inline void ChunksAroundBlock(World* world, Chunk* chunk, LChunkP lP, RelP rP, LChunkP& a, LChunkP& b)
{
    Chunk* chunkA = Rebase(world, lP, rP + DIR_LEFT_DOWN_BACK).chunk;
    Chunk* chunkB = Rebase(world, lP, rP + DIR_RIGHT_UP_FRONT).chunk;

    a = chunkA == nullptr ? chunk->lcPos : chunkA->lcPos;
    b = chunkB == nullptr ? chunk->lcPos : chunkB->lcPos;
}

static void FlagChunkForUpdate(World* world, Chunk* chunk, LChunkP lP, RelP rP, bool modified = true)
{
    chunk->modified = modified;

    LChunkP a, b;
    ChunksAroundBlock(world, chunk, lP, rP, a, b);

    for (int z = a.z; z <= b.z; z++)
    {
        for (int y = a.y; y <= b.y; y++)
        {
            for (int x = a.x; x <= b.x; x++)
            {
                Chunk* adj = GetChunk(world, x, y, z);

                if (adj->state >= CHUNK_BUILDING)
                    adj->pendingUpdate = true;
            }
        }
    }
}

static inline void SetBlock(World* world, LWorldP wPos, Block block)
{
    if (wPos.y < 0 || wPos.y >= WORLD_BLOCK_HEIGHT) return;

    if (OverlapsBlock(world->player, wPos.x, wPos.y, wPos.z) && !IsPassable(world, block))
        return;
    
	LChunkP lP = LWorldToLChunkP(wPos);

	Chunk* chunk = GetChunk(world, lP);
	assert(chunk != nullptr);
    assert(!IsEdgeChunk(world, chunk));

	RelP rP = LWorldToRelP(wPos);

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

static void LoadGroup(GameState*, World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;

    if (!LoadGroupFromDisk(world, group))
        world->biomes[world->properties.biome].func(world, group);

    group->loaded = true;
}

static ChunkGroup* CreateChunkGroup(GameState* state, World* world, int lcX, int lcZ, int cX, int cZ)
{
    int index = GroupIndex(world, lcX, lcZ);
	ChunkGroup* group = world->groups[index];

	if (group == nullptr)
	{
        group = world->groupPool.Get();
        memset(group, 0, sizeof(ChunkGroup));
        
        group->pos = ivec3(cX, 0, cZ);
        group->active = true;

        for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
        {
            Chunk* chunk = group->chunks + i;
            chunk->lcPos = ivec3(lcX, i, lcZ);
            chunk->lwPos = LChunkToLWorldP(chunk->lcPos);
        }

        QueueAsync(state, LoadGroup, world, group);
		world->groups[index] = group;
	}

	return group;
}

static void DestroyGroup(World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
        DestroyChunkMeshes(group->chunks + i);

    RemoveFromRegion(world, group);
    world->groupPool.Return(group);
}

// To allow "infinite" terrain, the world is always located near the origin.
// This function fills the world near the origin based on the reference
// world position within the world.
static void ShiftWorld(GameState* state, World* world)
{
    // Return all chunks in the active area to the hash table.
	for (int i = 0; i < world->totalGroups; i++)
	{
		AddGroupToHash(world, world->groups[i]);
		world->groups[i] = nullptr;
	}

    vector<ivec4>& groupsToCreate = world->groupsToCreate;

    // Any existing groups that still belong in the active area will be pulled in to their
    // new position. Any that don't exist in the hash table will be created.
	for (int z = 0; z < world->size; z++)
	{
		for (int x = 0; x < world->size; x++)
        {
            int wX = world->ref.x + x;
            int wZ = world->ref.z + z;

			ChunkGroup* group = GroupFromHash(world, wX, wZ);

			if (group == nullptr)
                groupsToCreate.push_back(ivec4(x, z, wX, wZ));
			else 
			{
                for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
                {
                    Chunk* chunk = group->chunks + i;
    				chunk->lcPos = ivec3(x, chunk->lcPos.y, z);
                    chunk->lwPos = LChunkToLWorldP(chunk->lcPos);
                }

                world->groups[GroupIndex(world, x, z)] = group;
			}
		}
	}

    vec2 playerChunk = vec2(world->loadRange, world->loadRange);

    sort(groupsToCreate.begin(), groupsToCreate.end(), [playerChunk](auto a, auto b) 
    { 
        float distA = distance2(vec2(a.x, a.y), playerChunk);
        float distB = distance2(vec2(b.x, b.y), playerChunk);
        return distA < distB;
    });

    for (int i = 0; i < groupsToCreate.size(); i++)
    {
        // Encoded ivec4 values as x, y = local x, z and z, w = world x, z.
        ivec4 p = groupsToCreate[i];
        CreateChunkGroup(state, world, p.x, p.y, p.z, p.w);
    }

    groupsToCreate.clear();

    // Any remaining chunks in the hash table are outside of the loaded area range
    // and should be returned to the pool.
	for (int c = 0; c < GROUP_HASH_SIZE; c++)
	{
		ChunkGroup* group = world->groupHash[c];

        if (group != nullptr && !group->active)
            world->destroyQueue.push(group);

        world->groupHash[c] = nullptr;
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
        UpdateCameraVectors(state->camera);
        ShiftWorld(state, world);
    }
}

static void UpdateWorld(GameState* state, World* world, Camera* cam, Player* player)
{
    if (player->suspended)
    {
        LChunkP cP = LWorldToLChunkP(player->pos);

        if (!ChunkInsideGroup(cP.y) || GetGroup(world, cP.x, cP.y)->loaded)
        {
            ivec3 bP = BlockPos(player->pos);

            // Ensure there are no blocks the player will be inside upon being unsuspended.
            for (int i = -1; i < 2; i++)
                SetBlock(world, bP.x, bP.y + i, bP.z, BLOCK_AIR);

            player->suspended = false;
        }
    }

    if (!player->spawned)
    {
        LChunkP lP = ChunkToLChunkP(world->spawnGroup, world->ref);
        ChunkGroup* spawnGroup = GetGroup(world, lP.x, lP.z);

        if (spawnGroup->loaded)
            SpawnPlayer(state, world, player, world->pBounds);
    }
    else
    {
        if (world->buildCount == 0)
            CheckWorld(state, world, player);
    }

    if (world->buildCount == 0)
    {
        GetCameraPlanes(cam);
        GetVisibleChunks(world, cam);
    }

    ProcessVisibleChunks(state, world, state->renderer);

    queue<ChunkGroup*>& destroyQueue = world->destroyQueue;
    int destroyLim = 0;

    while (!destroyQueue.empty() && destroyLim++ < GROUP_DESTROY_LIMIT)
    {
        ChunkGroup* group = destroyQueue.front();
        destroyQueue.pop();

        if (group->loaded)
        {
            group->pendingDestroy = true;
            QueueAsync(state, SaveGroup, world, group, DestroyGroup);
        }
        else destroyQueue.push(group);
    }
}

static inline void UpdateWorldRef(World* world, ChunkP p)
{
    ivec3 ref;
    ref.x = p.x - world->loadRange;
    ref.z = p.z - world->loadRange;
    world->ref = ref;
}

static World* NewWorld(GameState* state, int loadRange, WorldConfig& config, World* existing = nullptr)
{
    World* world;

    if (existing == nullptr)
    {
        world = new World();

        // Load range worth of groups on each side plus the middle group.
        world->size = (loadRange * 2) + 1;

        CreateBiomes(world);

        world->totalGroups = Square(world->size);
        world->groups = new ChunkGroup*[world->totalGroups]();

        world->visibleChunks.reserve(world->totalGroups * WORLD_CHUNK_HEIGHT);
        world->groupsToCreate.reserve(world->totalGroups);

        world->loadRange = loadRange;

        float min = (float)(loadRange * CHUNK_SIZE_H);
        float max = min + CHUNK_SIZE_H;

        world->pBounds = NewRect(vec3(min, 0.0f, min), vec3(max, 0.0f, max));

        CreateBlockData(state, world->blockData);

        if (!LoadWorldFileData(state, world))
        {
            srand((uint32_t)time(0));
            world->properties = { rand(), config.radius, BIOME_GRASSY };
        }

        world->blockToSet = BLOCK_GRASS;
        InitializeCriticalSection(&world->regionCS);
        InitializeConditionVariable(&world->regionsEmpty);
    }
    else 
    {
        world = existing;

        // The callback method skips saving the chunk if it is modified. 
        // We don't want to save any chunks here.
        for (int i = 0; i < world->totalGroups; i++)
        {
            DestroyGroup(world, world->groups[i]);
            world->groups[i] = nullptr;
        }

        world->properties.seed = rand();
        world->properties.radius = config.infinite ? INT_MAX : config.radius;
        world->properties.biome = config.biome;
        world->properties.homePos = {};
    }

    world->falloffRadius = world->properties.radius - (CHUNK_SIZE_H * 2);

    world->spawnGroup = ivec3(0, 0, 0);
    UpdateWorldRef(world, world->spawnGroup);
    ShiftWorld(state, world);

    return world;
}

static void RegenerateWorld(GameState* state, World* world, WorldConfig& config)
{
    DeleteDirectory(world->savePath);
    world = NewWorld(state, world->loadRange, config, world);
}
