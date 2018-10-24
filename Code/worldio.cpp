//
// Jason Bricco
//

static inline int RegionIndex(int x, int y, int z)
{
	return x + REGION_SIZE * (y + REGION_SIZE * z);
}

static inline int RegionIndex(ivec3 p)
{
	return RegionIndex(p.x, p.y, p.z);
}

static bool LoadRegionFile(World* world, RegionPos p)
{
	char path[MAX_PATH];
    sprintf(path, "%s\\%i%i%i.txt", world->savePath, p.x, p.y, p.z);

	if (!PathFileExists(path))
		return false;

	HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        
    if (file == INVALID_HANDLE_VALUE)
    {
        Print("An error occurred while loading region %i, %i, %i: %s\n", p.x, p.y, p.z, GetLastErrorText().c_str());
        return false;
    }

    vector<uint16_t> data;

    if (!ReadFile(file, data.data(), INT_MAX, NULL, NULL))
    	Print("Failed to load region file. Error: %s\n", GetLastErrorText().c_str());

    bool first = true;

    for (int i = 0; i < data.size(); i++)
    {
    	if (first)
    	{

    	}
    }

    // TODO: Insert into region map. We'll need to create the serialized chunks too.
    // world->regions.insert(make_pair(p, ))

    CloseHandle(file);

    return true;
}

static void SaveRegions(World* world)
{
	RegionMap& map = world->regions;

	for (auto it = map.begin(); it != map.end(); it++)
	{
		RegionPos p = it->first;
		Region start = it->second;

		char path[MAX_PATH];
    	sprintf(path, "%s\\%i%i%i.txt", world->savePath, p.x, p.y, p.z);

		HANDLE file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
		    Print("An error occurred while saving region %i, %i, %i: %s\n", p.x, p.y, p.z, GetLastErrorText().c_str());
		    return;
		}

		for (int z = 0; z < REGION_SIZE; z++)
		{
			for (int y = 0; y < REGION_SIZE; y++)
			{
				for (int x = 0; x < REGION_SIZE; x++)
				{
					int index = RegionIndex(x, y, z);
					SerializedChunk* chunk = start + index;

					if (chunk->modified)
					{
						DWORD bytesToWrite = sizeof(uint16_t) * (DWORD)chunk->data.size();
	    				DWORD bytesWritten;

	    				if (!WriteFile(file, chunk->data.data(), bytesToWrite, &bytesWritten, NULL))
			    		{
			        		Print("Failed to save chunk. Error: %s\n", GetLastErrorText().c_str());
			        		continue;
			    		}

			    		assert(bytesWritten == bytesToWrite);

			    		if (SetFilePointer(file, 0l, nullptr, FILE_END) == INVALID_SET_FILE_POINTER)
	    					Print("Failed to set the file pointer to the end of file.\n");

	    				chunk->modified = false;
					}
				}
			}
		}

		CloseHandle(file);
	}
}

static void LoadChunk(World* world, Chunk* chunk)
{
	ChunkPos p = chunk->cPos;
    RegionPos regionP = ChunkToRegionPos(p);

    auto it = world->regions.find(regionP);
    bool found = true;

    if (it == world->regions.end())
    {
    	if (LoadRegionFile(world, regionP))
    	{
    		it = world->regions.find(regionP);
    		assert(it != world->regions.end());
    	}
    	else found = false;
    }

    if (found)
    {
        Region region = it->second;

        ivec3 local = ivec3(p.x & REGION_MASK, p.y & REGION_MASK, p.z & REGION_MASK);
        int offset = RegionIndex(local);

        assert(offset >= 0 && offset < REGION_SIZE_3);
        
        SerializedChunk* chunkData = region + offset;

        int i = 0; 
        int loc = 0;

        while (i < CHUNK_SIZE_3)
        {
            uint16_t count = chunkData->data[loc++];
            Block block = chunkData->data[loc++];

            for (int j = 0; j < count; j++)
                chunk->blocks[i++] = block;
        }
    }
    else
    {
        world->regions.insert(make_pair(regionP, Calloc<SerializedChunk>(REGION_SIZE_3)));
        GenerateChunkTerrain(world, chunk);
    }

    chunk->state = CHUNK_LOADED;
}

static void SaveChunk(World* world, Chunk* chunk)
{
	assert(chunk->modified);

    ChunkPos p = chunk->cPos;

    RegionPos regionP = ChunkToRegionPos(p);

    auto it = world->regions.find(regionP);
    assert(it != world->regions.end());

    Region region = it->second;

	ivec3 local = ivec3(p.x & REGION_MASK, p.y & REGION_MASK, p.z & REGION_MASK);
    int offset = RegionIndex(local);

    assert(offset >= 0 && offset < REGION_SIZE_3);

    SerializedChunk* store = region + offset;
    store->data.clear();
    store->modified = true;
    auto& data = store->data;

    data.push_back((uint16_t)offset);

    Block currentBlock = chunk->blocks[0];
    uint16_t count = 1;

    for (int i = 1; i < CHUNK_SIZE_3; i++)
    {
        Block block = chunk->blocks[i];

        if (block != currentBlock)
        {
            data.push_back(count);
            data.push_back(currentBlock);
            count = 1;
            currentBlock = block;
        }
        else count++;

        if (i == CHUNK_SIZE_3 - 1)
        {
            data.push_back(count);
            data.push_back(currentBlock);
        }
    }

    data.push_back(USHRT_MAX);
    chunk->modified = false;
}

static void SaveWorld(World* world)
{
    char path[MAX_PATH];
    sprintf(path, "%s\\WorldData.txt", world->savePath);

    WriteBinary(path, (char*)&world->seed, sizeof(int));

    for (int i = 0; i < world->totalChunks; i++)
    {
        Chunk* chunk = world->chunks[i];

        if (chunk->modified)
            SaveChunk(world, chunk);
    }
}
