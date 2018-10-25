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

static bool LoadRegionFile(World* world, RegionPos p, RegionMap::iterator* it)
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

    Region region = Calloc<SerializedChunk>(REGION_SIZE_3);

    while (true)
    {
        DWORD bytesRead;
        uint16_t position;

        if (!ReadFile(file, &position, sizeof(uint16_t), &bytesRead, NULL))
        {
            Print("Failed to read chunk position. Error: %s\n", GetLastErrorText().c_str());
            return false;
        }

        if (bytesRead == 0) break;

        uint16_t items;

        if (!ReadFile(file, &items, sizeof(uint16_t), &bytesRead, NULL))
        {
            Print("Failed to read chunk items count. Error: %s\n", GetLastErrorText().c_str());
            return false;
        }

        if (bytesRead == 0) break;

        SerializedChunk* chunk = region + position;
        chunk->Reserve(items);

        if (!ReadFile(file, chunk->data, items * sizeof(uint16_t), &bytesRead, NULL))
        {
        	Print("Failed to load serialized chunk data. Error: %s", GetLastErrorText().c_str());
            Print("Tried to read %i items\n", items);
            return false;
        }

        chunk->size = items;

        if (bytesRead == 0) break;
    }

    *it = world->regions.insert(make_pair(p, region)).first;

    CloseHandle(file);
    return true;
}

static void SaveRegions(World* world)
{
	RegionMap& map = world->regions;

	for (auto it = map.begin(); it != map.end();)
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

					DWORD bytesToWrite = sizeof(uint16_t) * (DWORD)chunk->size;
    				DWORD bytesWritten;

    				if (!WriteFile(file, chunk->data, bytesToWrite, &bytesWritten, NULL))
		    		{
		        		Print("Failed to save chunk. Error: %s\n", GetLastErrorText().c_str());
		        		continue;
		    		}

		    		assert(bytesWritten == bytesToWrite);

		    		if (SetFilePointer(file, 0l, nullptr, FILE_END) == INVALID_SET_FILE_POINTER)
    					Print("Failed to set the file pointer to the end of file.\n");
				}
			}
		}

        ivec3 diff = p - world->playerRegion;

        if (abs(diff.x) > 1 || abs(diff.y) > 1 || abs(diff.z) > 1)
        {
            it = world->regions.erase(it);
            Print("Region at %i, %i, %i is too far. Removing.\n", p.x, p.y, p.z);
        }
        else it++;

        Print("Region at %i, %i, %i saved successfully!\n", p.x, p.y, p.z);
		CloseHandle(file);
	}
}

static void LoadChunk(World* world, Chunk* chunk)
{
	ChunkPos p = chunk->cPos;
    RegionPos regionP = ChunkToRegionPos(p);

    auto it = world->regions.find(regionP);

    if (it == world->regions.end())
    {
        world->regionMutex.lock();
        bool regionLoaded = LoadRegionFile(world, regionP, &it);
        world->regionMutex.unlock();

    	if (!regionLoaded)
            it = world->regions.insert(make_pair(regionP, Calloc<SerializedChunk>(REGION_SIZE_3))).first;
    }

    Region region = it->second;

    ivec3 local = ivec3(p.x & REGION_MASK, p.y & REGION_MASK, p.z & REGION_MASK);
    int offset = RegionIndex(local);

    assert(offset >= 0 && offset < REGION_SIZE_3);
    
    SerializedChunk* chunkData = region + offset;

    if (chunkData->size > 0)
    {
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
    else GenerateChunkTerrain(world, chunk);

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
    store->Clear();

    store->Add((uint16_t)offset);

    // Holds the number of elements we will write. We won't know this value until 
    // after the RLE compression, but should reserve its position in the data.
    store->Add(0);

    Block currentBlock = chunk->blocks[0];
    uint16_t count = 1;

    for (int i = 1; i < CHUNK_SIZE_3; i++)
    {
        Block block = chunk->blocks[i];

        if (block != currentBlock)
        {
            store->Add(count);
            store->Add(currentBlock);
            count = 1;
            currentBlock = block;
        }
        else count++;

        if (i == CHUNK_SIZE_3 - 1)
        {
            store->Add(count);
            store->Add(currentBlock);
        }
    }

    store->data[1] = (uint16_t)(store->size - 2);
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
