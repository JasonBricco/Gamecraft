//
// Jason Bricco
//

static inline int RegionIndex(int x, int z)
{
    return z * REGION_SIZE + x;
}

static inline int RegionIndex(ivec3 p)
{
    return RegionIndex(p.x, p.z);
}

static bool LoadRegionFile(World* world, RegionPos p, RegionMap::iterator* it)
{
    Print("Loading region at %i, %i\n", p.x, p.z);

    char path[MAX_PATH];
    sprintf(path, "%s\\%i%i.txt", world->savePath, p.x, p.z);

    if (!PathFileExists(path))
        return false;

    HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        
    if (file == INVALID_HANDLE_VALUE)
    {
        Print("An error occurred while loading region %i, %i, %i: %s\n", p.x, p.y, p.z, GetLastErrorText().c_str());
        return false;
    }

    Region region = {};
    region.chunks = (SerializedChunk*)calloc(REGION_SIZE_3, sizeof(SerializedChunk));
    region.hasData = true;

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
        assert(position >= 0 && position < REGION_SIZE_3);

        uint16_t items;

        if (!ReadFile(file, &items, sizeof(uint16_t), &bytesRead, NULL))
        {
            Print("Failed to read chunk items count. Error: %s\n", GetLastErrorText().c_str());
            return false;
        }

        if (bytesRead == 0) break;

        SerializedChunk* chunk = region.chunks + position;

        chunk->Reserve(items + 2);
        chunk->Add(position);
        chunk->Add(items);

        if (!ReadFile(file, chunk->data + 2, items * sizeof(uint16_t), &bytesRead, NULL))
        {
            Print("Failed to load serialized chunk data. Error: %s", GetLastErrorText().c_str());
            Print("Tried to read %i items\n", items);
            return false;
        }

        chunk->size += items;

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
        Region region = it->second;

        if (!region.hasData)
        {
            it++;
            continue;
        }

        char path[MAX_PATH];
        sprintf(path, "%s\\%i%i.txt", world->savePath, p.x, p.z);

        HANDLE file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file == INVALID_HANDLE_VALUE)
        {
            Print("An error occurred while saving region %i, %i: %s\n", p.x, p.z, GetLastErrorText().c_str());
            return;
        }

        for (int z = 0; z < REGION_SIZE; z++)
        {
            for (int x = 0; x < REGION_SIZE; x++)
            {
                int index = RegionIndex(x, z);
                assert(index >= 0 && index < REGION_SIZE_3);

                SerializedChunk* chunk = region.chunks + index;

                if (chunk->size > 0)
                {
                    DWORD bytesToWrite = sizeof(uint16_t) * chunk->size;
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

        if (abs(diff.x) > 1 || abs(diff.z) > 1)
        {
            it = world->regions.erase(it);
            Print("Region at %i, %i is too far. Removing.\n", p.x, p.z);
        }
        else it++;

        Print("Region at %i, %i saved successfully!\n", p.x, p.z);
        CloseHandle(file);
    }
}

static void DeleteRegions(World* world)
{
    RegionMap& map = world->regions;

    for (auto it = map.begin(); it != map.end(); it++)
    {
        Region region = it->second;
        assert(region.chunks != nullptr);
        free(region.chunks);
    }

    map.clear();
}

static bool LoadChunkFromDisk(World* world, Chunk* chunk)
{
    ChunkPos p = chunk->cPos;
    RegionPos regionP = ChunkToRegionPos(p);

    WaitForSingleObject(world->regionMutex, INFINITE);
    auto it = world->regions.find(regionP);

    if (it == world->regions.end())
    {
        bool regionLoaded = LoadRegionFile(world, regionP, &it);

        if (!regionLoaded)
        {
            Region region = {};
            region.chunks = (SerializedChunk*)calloc(REGION_SIZE_3, sizeof(SerializedChunk));
            it = world->regions.insert(make_pair(regionP, region)).first;
        }
    }

    ReleaseMutex(world->regionMutex);

    Region region = it->second;
    assert(RegionIsValid(region));

    ivec3 local = ivec3(p.x & REGION_MASK, 0, p.z & REGION_MASK);
    int offset = RegionIndex(local);

    assert(offset >= 0 && offset < REGION_SIZE_3);
    
    SerializedChunk* chunkData = region.chunks + offset;

    if (chunkData->size == 0)
        return false;

    int i = 0; 
    int loc = 2;

    // If every block in the chunk is the same, the saved count will be 65536 
    // but wrap to 0 as uint16_t's max is 65535. If we read in a 0, 
    // interpret it to be that every block in this chunk is the same.
    if (chunkData->data[loc] == 0)
        FillChunk(chunk, chunkData->data[loc + 1]);
    else
    {
        while (i < CHUNK_SIZE_3)
        {
            int count = chunkData->data[loc++];
            Block block = chunkData->data[loc++];

            for (int j = 0; j < count; j++)
                chunk->blocks[i++] = block;
        }
    }

    return true;
}

static void SaveChunk(World* world, Chunk* chunk)
{
    assert(chunk->modified);

    ChunkPos p = chunk->cPos;

    RegionPos regionP = ChunkToRegionPos(p);

    auto it = world->regions.find(regionP);
    assert(it != world->regions.end());

    Region& region = it->second;
    assert(RegionIsValid(region));

    ivec3 local = ivec3(p.x & REGION_MASK, 0, p.z & REGION_MASK);
    int offset = RegionIndex(local);

    assert(offset >= 0 && offset < REGION_SIZE_3);

    SerializedChunk* store = region.chunks + offset;
    region.hasData = true;
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
