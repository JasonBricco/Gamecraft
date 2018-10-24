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

    uint16_t position;

    if (!ReadFile(file, &position, sizeof(uint16_t), NULL, NULL))
        Print("Failed to read chunk position. Error: %s\n", GetLastErrorText().c_str());

    uint16_t items;

    if (!ReadFile(file, &items, sizeof(uint16_t), NULL, NULL))
        Print("Failed to read chunk items count. Error: %s\n", GetLastErrorText().c_str());

    Region region = Calloc<SerializedChunk>(REGION_SIZE_3);
    SerializedChunk* chunk = region + position;

    if (!ReadFile(file, chunk->data.data(), items * sizeof(uint16_t), NULL, NULL))
    	Print("Failed to load serialized chunk data. Error: %s\n", GetLastErrorText().c_str());

    world->regions.insert(make_pair(p, chunk));

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
        world->regionMutex.lock();
        bool regionLoaded = LoadRegionFile(world, regionP);
        world->regionMutex.unlock();

    	if (regionLoaded)
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

    // Holds the number of elements we will write. We won't know this value until 
    // after the RLE compression, but should reserve its position in the data.
    data.push_back(0);

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

    data[1] = (uint16_t)data.size() - 2;
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

    SaveRegions(world);
}
