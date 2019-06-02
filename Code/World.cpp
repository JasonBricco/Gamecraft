//
// Gamecraft
//

static inline bool BlockInsideChunkH(int x, int z)
{
    return x >= 0 && x < CHUNK_SIZE_H && z >= 0 && z < CHUNK_SIZE_H;
}

static inline bool BlockInsideChunk(int x, int y, int z)
{
    return BlockInsideChunkH(x, z) && y >= 0 && y < CHUNK_SIZE_V;
}

static inline bool BlockInsideChunk(RelP p)
{
    return BlockInsideChunk(p.x, p.y, p.z);
}

static inline bool BlockInsideGroup(int x, int y, int z)
{
    return x >= 0 && x < CHUNK_SIZE_H && y >= 0 && y < WORLD_BLOCK_HEIGHT && z >= 0 && z < CHUNK_SIZE_H;
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

static inline int RelToLWorldP(Chunk* chunk, int rY)
{
    return chunk->lwPos.y + rY;
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

static inline Biome& GetCurrentBiome(World* world)
{
    return world->biomes[world->properties.biome];
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

static inline ChunkGroup* GetGroup(World* world, ivec3 p)
{
    return GetGroup(world, p.x, p.z);
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

static inline int BlockIndex(int x, int y, int z)
{
    return x + CHUNK_SIZE_H * (y + CHUNK_SIZE_V * z);
}

static inline Block GetBlock(Chunk* chunk, int rX, int rY, int rZ)
{
    assert(rY >= 0 && rY < CHUNK_SIZE_V);
    int index = BlockIndex(rX, rY, rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    Block block = chunk->blocks[index];
    assert(block >= 0 && block < BLOCK_COUNT);
    return block;
}

static inline Block GetBlock(Chunk* chunk, RelP pos)
{
    return GetBlock(chunk, pos.x, pos.y, pos.z);
}

static inline Block GetBlock(ChunkGroup* group, int rX, int lwY, int rZ)
{
    int lcY = lwY >> CHUNK_V_BITS;
    Chunk* chunk = group->chunks + lcY;
    return GetBlock(chunk, rX, lwY & CHUNK_V_MASK, rZ);
}

static inline NeighborBlocks GetNeighborBlocks(World* world, Chunk* chunk, int x, int y, int z)
{
    LChunkP lcP = chunk->lcPos;

    RebasedPos up = { chunk, x, y + 1, z };
    RebasedPos down = { chunk, x, y - 1, z };
    RebasedPos front = { chunk, x, y, z + 1 };
    RebasedPos back = { chunk, x, y, z - 1 };
    RebasedPos left = { chunk, x - 1, y, z };
    RebasedPos right = { chunk, x + 1, y, z };

    if (up.rY == CHUNK_SIZE_V)
    {
        LChunkP tar = lcP + DIR_UP;

        if (tar.y >= WORLD_CHUNK_HEIGHT)
        {
            up.chunk = nullptr;
            up.rY = 1;
        }
        else
        {
            up.chunk = GetChunk(world, tar);
            up.rY = 0;
        }
    }

    if (down.rY < 0)
    {
        LChunkP tar = lcP + DIR_DOWN;

        if (tar.y < 0)
        {
            down.chunk = nullptr;
            down.rY = -1;
        }
        else
        {
            down.chunk = GetChunk(world, tar);
            down.rY = CHUNK_V_MASK;
        }
    }

    if (right.rX == CHUNK_SIZE_H)
    {
        right.chunk = GetChunk(world, lcP + DIR_RIGHT);
        right.rX = 0;
    }

    if (left.rX < 0)
    {
        left.chunk = GetChunk(world, lcP + DIR_LEFT);
        left.rX = CHUNK_H_MASK;
    }

    if (front.rZ == CHUNK_SIZE_H)
    {
        front.chunk = GetChunk(world, lcP + DIR_FRONT);
        front.rZ = 0;
    }

    if (back.rZ < 0)
    {
        back.chunk = GetChunk(world, lcP + DIR_BACK);
        back.rZ = CHUNK_H_MASK;
    }

    return { up, down, front, back, left, right };
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

static inline Block GetBlockSafe(RebasedPos p)
{
    if (p.chunk == nullptr)
    {
        assert(p.rY == -1 || p.rY == 1);
        return p.rY == -1 ? BLOCK_STONE : BLOCK_AIR;
    }

    return GetBlock(p.chunk, p.rX, p.rY, p.rZ);
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, int rX, int rY, int rZ)
{
    RebasedPos p = Rebase(world, chunk->lcPos, rX, rY, rZ);
    return GetBlockSafe(Rebase(world, chunk->lcPos, rX, rY, rZ));
}

static inline Block GetBlockSafe(World* world, Chunk* chunk, RelP p)
{
    return GetBlockSafe(world, chunk, p.x, p.y, p.z);
}

static Chunk* GetRelative(World* world, int lwX, int lwY, int lwZ, RelP& rel)
{
    assert(lwY >= 0 && lwY < WORLD_BLOCK_HEIGHT);

    LChunkP lcPos = LWorldToLChunkP(lwX, lwY, lwZ);
    ChunkGroup* group = GetGroup(world, lcPos.x, lcPos.z);

    assert(group != nullptr && group->state != GROUP_DEFAULT);

    rel = LWorldToRelP(lwX, lwY, lwZ);

    return GetChunk(group, lcPos.y);
}

static Block GetBlock(World* world, int lwX, int lwY, int lwZ)
{
    if (lwY < 0) return BLOCK_KILL_ZONE;

    if (lwY >= WORLD_BLOCK_HEIGHT || !BlockInsideWorldH(world, lwX, lwZ))
        return BLOCK_AIR;

    LChunkP lcPos = LWorldToLChunkP(lwX, lwY, lwZ);
    ChunkGroup* group = GetGroup(world, lcPos.x, lcPos.z);

    if (group == nullptr || group->state == GROUP_DEFAULT)
        return BLOCK_AIR;

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
    int index = BlockIndex(rX, rY, rZ);
    assert(index >= 0 && index < CHUNK_SIZE_3);
    chunk->blocks[index] = block;
}

static inline void SetBlock(Chunk* chunk, RelP pos, Block block)
{
    SetBlock(chunk, pos.x, pos.y, pos.z, block);
}

static inline void SetBlock(ChunkGroup* group, int rX, int lwY, int rZ, Block block)
{
    int lcY = lwY >> CHUNK_V_BITS;
    Chunk* chunk = group->chunks + lcY;
    SetBlock(chunk, rX, lwY & CHUNK_V_MASK, rZ, block);
}

static void FlagChunkForUpdate(World* world, Chunk* chunk, LChunkP lP, RelP rP, bool modified = true)
{
    chunk->modified = modified;

    Chunk* chunkA = Rebase(world, lP, rP + DIR_LEFT_DOWN_BACK).chunk;
    Chunk* chunkB = Rebase(world, lP, rP + DIR_RIGHT_UP_FRONT).chunk;

    LChunkP a = chunkA == nullptr ? chunk->lcPos : chunkA->lcPos;
    LChunkP b = chunkB == nullptr ? chunk->lcPos : chunkB->lcPos;

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

    // If the chunk isn't built, we don't know its number of vertices.
    // Setting the block in that case would be unsafe and could overflow the chunk.
    if (chunk->state < CHUNK_NEEDS_FILL)
        return;

	RelP rP = LWorldToRelP(wPos);
    int index = BlockIndex(rP.x, rP.y, rP.z);

    if (chunk->blocks[index] != block)
    {
        chunk->blocks[index] = block;

        if (ChunkOverflowed(world, chunk, rP.x, rP.y, rP.z))
            chunk->blocks[index] = BLOCK_AIR;
        else
        {
            PlaySound(GetSetSound(world, block));
            RecomputeLight(world, chunk, rP.x, rP.y, rP.z);
            FlagChunkForUpdate(world, chunk, lP, rP);
        }
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

static int CountNonAirBlocks(Chunk* chunk)
{
    int count = 0;

    for (int i = 0; i < CHUNK_SIZE_3; i++)
    {
        if (chunk->blocks[i] != BLOCK_AIR)
            count++;
    }

    return count;
}

static void LoadGroup(GameState*, World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;

    if (!LoadGroupFromDisk(world, group))
        world->biomes[world->properties.biome].func(world, group);

    ComputeSurface(group);
    group->state = GROUP_LOADED;
}

static void OnGroupLoaded(GameState*, World* world, void*)
{
    world->workCount--;
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
            chunk->group = group;
        }

        world->workCount++;
        QueueAsync(state, LoadGroup, world, group, OnGroupLoaded);

		world->groups[index] = group;
	}

	return group;
}

static void DestroyGroup(GameState* state, World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        DestroyMesh(chunk->mesh);
        ReturnChunkMesh(state->renderer, chunk);
    }

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
        UpdateViewMatrix(state->camera);
        ShiftWorld(state, world);
    }
}

static inline bool HasBackgroundWork(World* world)
{
    return world->workCount > 0;
}

static void UpdateWorld(GameState* state, World* world, Camera* cam, Player* player)
{
    if (player->suspended)
    {
        LChunkP cP = LWorldToLChunkP(player->pos);

        if (!ChunkInsideGroup(cP.y) || GetGroup(world, cP.x, cP.y)->state != GROUP_DEFAULT)
        {
            ivec3 bP = BlockPos(player->pos);

            // Ensure there are no blocks the player will be inside upon being unsuspended.
            for (int i = 0; i < 2; i++)
                SetBlock(world, bP.x, bP.y + i, bP.z, BLOCK_AIR);

            player->suspended = false;
        }
    }

    if (!player->spawned)
    {
        LChunkP lP = ChunkToLChunkP(world->spawnGroup, world->ref);
        ChunkGroup* spawnGroup = GetGroup(world, lP.x, lP.z);

        if (spawnGroup->state != GROUP_DEFAULT)
            SpawnPlayer(state, world, player, world->pBounds);
    }
    else 
    {
        if (!HasBackgroundWork(world))
            CheckWorld(state, world, player);
    }

    GetCameraPlanes(cam);

    WorldRenderUpdate(state, world, cam);
    PrepareWorldRender(state, world, state->renderer);

    auto& destroyQueue = world->destroyQueue;
    int destroyLim = 0;

    while (!destroyQueue.empty() && destroyLim++ < GROUP_DESTROY_LIMIT)
    {
        ChunkGroup* group = destroyQueue.front();
        destroyQueue.pop();
        group->pendingDestroy = true;
        QueueAsync(state, SaveGroup, world, group, DestroyGroup);
    }
}

static void TeleportHome(GameState* state, World* world, Player* player)
{
    WorldLocation home = world->properties.homePos;
    TeleportPlayer(state, player, world->properties.homePos);
}

static void SetHomePos(World* world, Player* player)
{
    LWorldP lW = BlockPos(player->pos);
    lW.y++;
    world->properties.homePos = { LWorldToWorldP(world, player->pos), lW };
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

        CreateBiomes(state, world);

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
            world->properties = { rand(), config.radius, BIOME_FOREST };

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
            DestroyGroup(state, world, world->groups[i]);
            world->groups[i] = nullptr;
        }

        world->properties.seed = rand();
        world->properties.radius = config.infinite ? INT_MAX : config.radius;
        world->properties.biome = config.biome;
        world->properties.homePos = {};
    }

    ResetEnvironment(state, world);
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

static void RegenerateWorldCallback(GameState* state, World* world)
{
    RegenerateWorld(state, world, *state->pendingConfig);
    world->player->spawned = false;
    world->player->velocity.y = 0.0f;
}
