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
		SerializedChunk* chunk = region + i;

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
