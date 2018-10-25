//
// Jason Bricco
//

static bool ChunkIsValid(World* world, Chunk* chunk)
{
    if (chunk == nullptr) return true;

    LChunkPos lP = chunk->lcPos;

    int limX = world->sizeH, limY = world->sizeV;

    if (lP.x < 0 || lP.y < 0 || lP.z < 0 || lP.x >= limX || lP.y >= limY || lP.z >= limX)
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
