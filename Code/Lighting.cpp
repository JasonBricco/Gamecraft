//
// Gamecraft
//

// TODO: Fix a light propagation bug. If you create an enclosed room, light will not update
// when breaking the wall; it only updates when breaking the ceiling.

static inline void ComputeSurfaceAt(ChunkGroup* group, int x, int z)
{
    int index = z * CHUNK_SIZE_H + x;

    for (int y = WORLD_BLOCK_HEIGHT - 2; y >= 0; y--)
    {
        if (GetBlock(group, x, y, z) != BLOCK_AIR)
        {
            group->surface[index] = (uint8_t)(y + 1);
            break;
        }
    }
}

static inline void ComputeSurface(ChunkGroup* group)
{
    for (int z = 0; z < CHUNK_SIZE_H; z++) 
    {
        for (int x = 0; x < CHUNK_SIZE_H; x++)
            ComputeSurfaceAt(group, x, z);
    }
}

static inline int ComputeMaxY(World* world, ChunkGroup* group, int x, int z, int surface)
{
    LChunkP lcP = group->chunks->lcPos;

    RebasedGroupPos front = { group, x, z + 1 };
    RebasedGroupPos back = { group, x, z - 1 };
    RebasedGroupPos left = { group, x - 1, z };
    RebasedGroupPos right = { group, x + 1, z };

    if (right.rX == CHUNK_SIZE_H)
    {
        right.group = GetGroup(world, lcP + DIR_RIGHT);
        right.rX = 0;
    }

    if (left.rX < 0)
    {
        left.group = GetGroup(world, lcP + DIR_LEFT);
        left.rX = CHUNK_H_MASK;
    }

    if (front.rZ == CHUNK_SIZE_H)
    {
        front.group = GetGroup(world, lcP + DIR_FRONT);
        front.rZ = 0;
    }

    if (back.rZ < 0)
    {
        back.group = GetGroup(world, lcP + DIR_BACK);
        back.rZ = CHUNK_H_MASK;
    }

    surface = Max(surface, front.group->surface[front.rZ * CHUNK_SIZE_H + front.rX]);
    surface = Max(surface, back.group->surface[back.rZ * CHUNK_SIZE_H + back.rX]);
    surface = Max(surface, left.group->surface[left.rZ * CHUNK_SIZE_H + left.rX]);
    surface = Max(surface, right.group->surface[right.rZ * CHUNK_SIZE_H + right.rX]);
    
    return surface;
}

static inline uint8_t GetSunlight(Chunk* chunk, int rX, int lwY, int rZ, int index)
{
    assert(BlockInsideChunkH(rX, rZ));

    if (lwY >= chunk->group->surface[rZ * CHUNK_SIZE_H + rX])
        return MAX_LIGHT;

    uint8_t light = chunk->sunlight[index];
    
    return light == 0 ? MIN_LIGHT : light;
}

static inline bool SetMaxSunlight(Chunk* chunk, int light, int lwY, RelP rel) 
{
    int index = BlockIndex(rel.x, rel.y, rel.z);
    int oldLight = GetSunlight(chunk, rel.x, lwY, rel.z, index);
    
    if (oldLight < light) 
    {
        chunk->sunlight[index] = (uint8_t)light;
        return true;
    }
    
    return false;
}

static inline bool SetMaxBlockLight(Chunk* chunk, int light, RelP rel) 
{
    int index = BlockIndex(rel.x, rel.y, rel.z);
    int oldLight = chunk->blockLight[index];
    
    if (oldLight < light) 
    {
        chunk->blockLight[index] = (uint8_t)light;
        return true;
    }
    
    return false;
}

static inline void ScatterSunlight(World* world, Queue<ivec3>& sunNodes, bool updateChunks)
{
    while (!sunNodes.Empty())
    {
        LWorldP node = sunNodes.Dequeue();

        RelP rel;
        Chunk* chunk = GetRelative(world, node.x, node.y, node.z, rel);

        int index = BlockIndex(rel.x, rel.y, rel.z);
        Block block = chunk->blocks[index];

        int light = GetSunlight(chunk, rel.x, node.y, rel.z, index) - GetLightStep(world, block);

        if (light <= MIN_LIGHT)
            continue;

        for (int i = 0; i < 6; i++)
        {
            LWorldP nextP = node + DIRS_3[i];

            if (nextP.y < 0 || nextP.y >= WORLD_BLOCK_HEIGHT)
                continue;

            Chunk* next = GetRelative(world, nextP.x, nextP.y, nextP.z, rel);
            Block adjBlock = GetBlock(next, rel);

            if (updateChunks && next->state >= CHUNK_BUILDING)
                next->pendingUpdate = true;

            if (!IsOpaque(world, adjBlock) && SetMaxSunlight(next, light, nextP.y, rel))
                sunNodes.Enqueue(ivec3(nextP.x, nextP.y, nextP.z));
        }
    }
}

static inline void RemoveSunlightNodes(World* world, Queue<ivec3>& sunNodes)
{
	Queue<ivec3> newNodes(MAX_LIGHT_NODES);

    while (!sunNodes.Empty())
    {
        LWorldP node = sunNodes.Dequeue();

        RelP rel;
        Chunk* chunk = GetRelative(world, node.x, node.y, node.z, rel);

        if (node.y >= chunk->group->surface[rel.z * CHUNK_SIZE_H + rel.x])
        {
        	newNodes.Enqueue(node);
        	continue;
        }

        int index = BlockIndex(rel.x, rel.y, rel.z);
        int light = GetSunlight(chunk, rel.x, node.y, rel.z, index) - 1;

        chunk->sunlight[index] = MIN_LIGHT;

        if (light <= MIN_LIGHT)
            continue;

        for (int i = 0; i < 6; i++)
        {
            LWorldP nextP = node + DIRS_3[i];

            if (nextP.y < 0 || nextP.y >= WORLD_BLOCK_HEIGHT)
                continue;

            Chunk* next = GetRelative(world, nextP.x, nextP.y, nextP.z, rel);
            Block adjBlock = GetBlock(next, rel);

            if (!IsOpaque(world, adjBlock))
            {
            	if (GetSunlight(next, rel.x, nextP.y, rel.z, BlockIndex(rel.x, rel.y, rel.z)) <= light)
            		sunNodes.Enqueue(ivec3(nextP.x, nextP.y, nextP.z));
            	else newNodes.Enqueue(ivec3(nextP.x, nextP.y, nextP.z));
            }

            if (next->state >= CHUNK_BUILDING)
                next->pendingUpdate = true;
        }
    }

    ScatterSunlight(world, newNodes, true);
}

static inline void ScatterBlockLight(World* world, Queue<ivec3>& lightNodes, bool updateChunks)
{
    while (!lightNodes.Empty())
    {
        LWorldP node = lightNodes.Dequeue();

        RelP rel;
        Chunk* chunk = GetRelative(world, node.x, node.y, node.z, rel);

        int index = BlockIndex(rel.x, rel.y, rel.z);
        Block block = chunk->blocks[index];

        int light = chunk->blockLight[index] - GetLightStep(world, block);

        if (light <= MIN_LIGHT)
            continue;

        for (int i = 0; i < 6; i++)
        {
            LWorldP nextP = node + DIRS_3[i];

            if (nextP.y < 0 || nextP.y >= WORLD_BLOCK_HEIGHT)
                continue;

            Chunk* next = GetRelative(world, nextP.x, nextP.y, nextP.z, rel);
            Block adjBlock = GetBlock(next, rel);

            if (updateChunks && next->state >= CHUNK_BUILDING)
                next->pendingUpdate = true;

            if (!IsOpaque(world, adjBlock) && SetMaxBlockLight(next, light, rel))
                lightNodes.Enqueue(ivec3(nextP.x, nextP.y, nextP.z));
        }
    }
}

static inline bool CheckForBlockLight(World* world, ChunkGroup* group, int x, int lwY, int z)
{
    Chunk* chunk = GetChunk(group, lwY >> CHUNK_V_BITS);
    int index = BlockIndex(x, lwY & CHUNK_V_MASK, z);

    int emission = GetLightEmitted(world, chunk->blocks[index]);

    if (emission > MIN_LIGHT)
    {
        chunk->blockLight[index] = (uint8_t)emission;
        return true;
    }

    return false;
}

static void SetLightNodes(World* world, ChunkGroup* group, Queue<ivec3>& sunNodes, Queue<ivec3>& lightNodes)
{
    LWorldP lwP = group->chunks->lwPos;

    for (int z = 0; z < CHUNK_SIZE_H; z++) 
    {
        int lwZ = lwP.z + z;

        for (int x = 0; x < CHUNK_SIZE_H; x++)
        {
            int surface = group->surface[z * CHUNK_SIZE_H + x];
            int maxY = Min(ComputeMaxY(world, group, x, z, surface), WORLD_BLOCK_HEIGHT - 1);

            int lwX = lwP.x + x;

            for (int lwY = 0; lwY < surface; lwY++)
            {
                if (CheckForBlockLight(world, group, x, lwY, z))
                    lightNodes.Enqueue(ivec3(lwX, lwY, lwZ));
            }

            for (int lwY = surface; lwY <= maxY; lwY++)
            {
                ivec3 wP = ivec3(lwX, lwY, lwZ);
                sunNodes.Enqueue(wP);

                if (CheckForBlockLight(world, group, x, lwY, z))
                    lightNodes.Enqueue(wP);
            }
        }
    }

    ScatterSunlight(world, sunNodes, false);
    ScatterBlockLight(world, lightNodes, false);
}

static void UpdateSunlight(World* world, LWorldP pos, Queue<ivec3>& sunNodes)
{
	for (int i = 0; i < 6; i++)
	{
	    LWorldP nextP = pos + DIRS_3[i];

	    if (nextP.y < 0 || nextP.y >= WORLD_BLOCK_HEIGHT)
	        continue;

	    sunNodes.Enqueue(nextP);
	}

	ScatterSunlight(world, sunNodes, true);
}

static void RecomputeSunlight(World* world, Chunk* chunk, int rX, int rY, int rZ, Queue<ivec3>& sunNodes)
{
    ChunkGroup* group = chunk->group;

    int lwX = chunk->lwPos.x + rX;
    int lwZ = chunk->lwPos.z + rZ;

    int surfaceIndex = rZ * CHUNK_SIZE_H + rX;
    int oldSurface =  group->surface[surfaceIndex];

    ComputeSurfaceAt(group, rX, rZ);

    int newSurface = group->surface[surfaceIndex];

    if (newSurface < oldSurface)
    {
        for (int lwY = newSurface; lwY <= oldSurface; lwY++)
            sunNodes.Enqueue(ivec3(lwX, lwY, lwZ));

        ScatterSunlight(world, sunNodes, true);
    }
    else if (newSurface > oldSurface)
    {
        for (int lwY = oldSurface; lwY <= newSurface; lwY++)
        {
            Chunk* tChunk = GetChunk(group, lwY >> CHUNK_V_BITS);
            tChunk->sunlight[BlockIndex(rX, lwY & CHUNK_V_MASK, rZ)] = MAX_LIGHT;
            sunNodes.Enqueue(ivec3(lwX, lwY, lwZ));
        }

        RemoveSunlightNodes(world, sunNodes);
    }
    else
    {
    	if (IsOpaque(world, GetBlock(chunk, rX, rY, rZ)))
    	{
    		chunk->sunlight[BlockIndex(rX, rY, rZ)] = MIN_LIGHT;
    		sunNodes.Enqueue(ivec3(lwX, chunk->lwPos.y + rY, lwZ));
    		RemoveSunlightNodes(world, sunNodes);
    	}
    	else UpdateSunlight(world, ivec3(lwX, chunk->lwPos.y + rY, lwZ), sunNodes);
    }
}

static inline void RemoveLightNodes(World* world, Queue<ivec3>& lightNodes)
{
	Queue<ivec3> newNodes(MAX_LIGHT_NODES);

    while (!lightNodes.Empty())
    {
        LWorldP node = lightNodes.Dequeue();

        RelP rel;
        Chunk* chunk = GetRelative(world, node.x, node.y, node.z, rel);

        int index = BlockIndex(rel.x, rel.y, rel.z);
        int light = chunk->blockLight[index] - 1;

        chunk->blockLight[index] = MIN_LIGHT;

        if (light <= MIN_LIGHT)
            continue;

        for (int i = 0; i < 6; i++)
        {
            LWorldP nextP = node + DIRS_3[i];

            if (nextP.y < 0 || nextP.y >= WORLD_BLOCK_HEIGHT)
                continue;

            Chunk* next = GetRelative(world, nextP.x, nextP.y, nextP.z, rel);
            Block adjBlock = GetBlock(next, rel);

            if (!IsOpaque(world, adjBlock))
            {
            	if (next->blockLight[BlockIndex(rel.x, rel.y, rel.z)] <= light)
            		lightNodes.Enqueue(ivec3(nextP.x, nextP.y, nextP.z));
            	else newNodes.Enqueue(ivec3(nextP.x, nextP.y, nextP.z));
            }

            if (GetLightEmitted(world, adjBlock) > MIN_LIGHT)
				newNodes.Enqueue(nextP);

			if (next->state >= CHUNK_BUILDING)
            	next->pendingUpdate = true;
        }
    }

    ScatterBlockLight(world, newNodes, true);
}

static void UpdateBlockLight(World* world, LWorldP pos, Queue<ivec3>& lightNodes)
{
	for (int i = 0; i < 6; i++)
	{
	    LWorldP nextP = pos + DIRS_3[i];

	    if (nextP.y < 0 || nextP.y >= WORLD_BLOCK_HEIGHT)
	        continue;

	    lightNodes.Enqueue(nextP);
	}

	ScatterBlockLight(world, lightNodes, true);
}

static void RecomputeBlockLight(World* world, Chunk* chunk, int rX, int rY, int rZ, Queue<ivec3>& lightNodes)
{
	int index = BlockIndex(rX, rY, rZ);
	int oldLight = chunk->blockLight[index];
	int emission = GetLightEmitted(world, chunk->blocks[index]);

	int lwX = chunk->lwPos.x + rX;
	int lwY = chunk->lwPos.y + rY;
    int lwZ = chunk->lwPos.z + rZ;

    if (emission < oldLight)
    {
    	chunk->blockLight[index] = MAX_LIGHT;
    	lightNodes.Enqueue(ivec3(lwX, lwY, lwZ));
    	RemoveLightNodes(world, lightNodes);
    }

    if (emission > MIN_LIGHT)
    {
    	chunk->blockLight[index] = (uint8_t)emission;
    	lightNodes.Enqueue(ivec3(lwX, lwY, lwZ));
    	ScatterBlockLight(world, lightNodes, true);
    }
    else UpdateBlockLight(world, ivec3(lwX, lwY, lwZ), lightNodes);
}

static void RecomputeLight(World* world, Chunk* chunk, int rX, int rY, int rZ)
{
    Queue<ivec3> sunNodes(262144);
    Queue<ivec3> lightNodes(262144);

    RecomputeSunlight(world, chunk, rX, rY, rZ, sunNodes);
    RecomputeBlockLight(world, chunk, rX, rY, rZ, lightNodes);
}
