//
// Gamecraft
//

static void MakeRect(Chunk* chunk, int startX, int startY, int startZ, int endX, int endY, int endZ, Block block)
{
	for (int z = startZ; z <= endZ; z++)
	{
		for (int y = startY; y <= endY; y++)
		{
			for (int x = startX; x <= endX; x++)
				SetBlock(chunk, x, y, z, block);
		}
	}
}

static void GenerateDungeon(World*, ChunkGroup* group)
{
	Chunk* chunk = group->chunks;
	int wallHeight = 10;
	int lim = CHUNK_SIZE_H - 1;

	// Floor.
	MakeRect(chunk, 0, 0, 0, lim, 0, lim, BLOCK_STONE);

	// Walls.
	MakeRect(chunk, 0, 0, 0, lim, wallHeight, 0, BLOCK_STONE);
	MakeRect(chunk, 0, 0, 0, 0, wallHeight, lim, BLOCK_STONE);
	MakeRect(chunk, 0, 0, lim, lim, wallHeight, lim, BLOCK_STONE);
	MakeRect(chunk, lim, 0, 0, lim, wallHeight, lim, BLOCK_STONE);
}
