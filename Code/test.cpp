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

static inline void AddColumnCollision(World* world, Player* player, int x, int z, int minY, int maxY);
static inline void SetBlock(World* world, int lwX, int lwY, int lwZ, Block block);

static void TestColumnCollision(World* world, Player* player)
{
    const int h = 32;

    for (int y = 0; y < 4; y++)
        SetBlock(world, h, y, h, BLOCK_STONE);
    
    AddColumnCollision(world, player, h, h, 0, 3);
    assert(player->possibleCollides.size() == 1);

    SetBlock(world, h, 0, h, BLOCK_STONE);
    SetBlock(world, h, 1, h, BLOCK_AIR);
    SetBlock(world, h, 2, h, BLOCK_STONE);
    SetBlock(world, h, 3, h, BLOCK_STONE);

    player->possibleCollides.clear();

    AddColumnCollision(world, player, h, h, 0, 3);
    assert(player->possibleCollides.size() == 1);

    player->possibleCollides.clear();

    SetBlock(world, h, 0, h, BLOCK_AIR);
    SetBlock(world, h, 1, h, BLOCK_STONE);
    SetBlock(world, h, 2, h, BLOCK_STONE);
    SetBlock(world, h, 3, h, BLOCK_STONE);

    AddColumnCollision(world, player, h, h, 0, 3);
    assert(player->possibleCollides.size() == 1);

    player->possibleCollides.clear();

    SetBlock(world, h, 0, h, BLOCK_STONE);
    SetBlock(world, h, 1, h, BLOCK_AIR);
    SetBlock(world, h, 2, h, BLOCK_AIR);
    SetBlock(world, h, 3, h, BLOCK_STONE);

    AddColumnCollision(world, player, h, h, 0, 3);
    assert(player->possibleCollides.size() == 2);

    player->possibleCollides.clear();

    SetBlock(world, h, 0, h, BLOCK_AIR);
    SetBlock(world, h, 1, h, BLOCK_STONE);
    SetBlock(world, h, 2, h, BLOCK_AIR);
    SetBlock(world, h, 3, h, BLOCK_STONE);

    AddColumnCollision(world, player, h, h, 0, 3);
    assert(player->possibleCollides.size() == 1);
}

#else

#define ChunkIsValid(world, chunk)
#define RegionIsValid(region)
#define TestColumnCollision(world, player)

#endif
