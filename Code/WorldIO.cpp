//
// Gamecraft
//

static inline int RegionIndex(int x, int y, int z)
{
    return x + REGION_SIZE * (y + WORLD_CHUNK_HEIGHT * z);
}

static inline int RegionIndex(ivec3 p)
{
    return RegionIndex(p.x, p.y, p.z);
}

static Region* LoadRegionFile(World* world, RegionP p)
{
    Region* region = world->regionPool.Get();
    region->pos = p;

    char path[MAX_PATH];
    sprintf(path, "%s\\%i%i.txt", world->savePath, p.x, p.z);

    if (!PathFileExists(path))
        return region;

    HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        
    if (file == INVALID_HANDLE_VALUE)
    {
        Print("An error occurred while loading region %i, %i, %i: %s\n", p.x, p.y, p.z, GetLastErrorText().c_str());
        return false;
    }

    region->hasData = true;

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

        List<uint16_t>* chunk = region->chunks + position;
        chunk->Reserve(items + 2);
        chunk->Add(position);
        chunk->Add(items);
        chunk->size += items;

        if (!ReadFile(file, chunk->items + 2, items * sizeof(uint16_t), &bytesRead, NULL))
        {
            Print("Failed to load serialized chunk data. Error: %s", GetLastErrorText().c_str());
            Print("Tried to read %i items\n", items);
            return false;
        }

        if (bytesRead == 0) break;
    }

    CloseHandle(file);
    return region;
}

static void SaveRegion(World* world, Region* region)
{
    if (!region->modified)
        return;

    assert(region->hasData);
    RegionP p = region->pos;

    char path[MAX_PATH];
    sprintf(path, "%s\\%i%i.txt", world->savePath, p.x, p.z);

    HANDLE file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
        Error("An error occurred while saving region %i, %i: %s\n", p.x, p.z, GetLastErrorText().c_str());
        return;
    }

    for (int z = 0; z < REGION_SIZE; z++)
    {
        for (int y = 0; y < WORLD_CHUNK_HEIGHT; y++)
        {
            for (int x = 0; x < REGION_SIZE; x++)
            {
                int index = RegionIndex(x, y, z);
                assert(index >= 0 && index < REGION_SIZE_3);

                List<uint16_t>& chunk = region->chunks[index];
                DWORD size = chunk.size;

                if (size > 0)
                {
                    DWORD bytesToWrite = sizeof(uint16_t) * size;
                    DWORD bytesWritten;

                    if (!WriteFile(file, chunk.items, bytesToWrite, &bytesWritten, NULL))
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
    }

    region->modified = false;
    CloseHandle(file);
}

static void SaveAllRegions(World* world)
{
    for (Region* region : world->regions)
        SaveRegion(world, region);
}

static RegionEntry GetRegion(World* world, RegionP pos)
{
    for (auto it = world->regions.begin(); it != world->regions.end(); it++)
    {
        if ((*it)->pos == pos)
            return it;
    }

    return world->regions.end();
}

static Region* GetOrLoadRegion(World* world, RegionP pos)
{
    RegionEntry entry = GetRegion(world, pos);
    Region* region;

    if (entry == world->regions.end())
    {
        region = LoadRegionFile(world, pos);
        world->regions.push_back(region);
    }
    else region = *entry;

    return region;
}

static bool LoadGroupFromDisk(World* world, ChunkGroup* group)
{
    ChunkP p = group->pos;
    RegionP regionP = ChunkToRegionP(p);

    EnterCriticalSection(&world->regionCS);

    Region* region = GetOrLoadRegion(world, regionP);
    region->activeCount++;

    LeaveCriticalSection(&world->regionCS);

    bool complete = true;

    for (int y = 0; y < WORLD_CHUNK_HEIGHT; y++)
    {
        ivec3 local = ivec3(p.x & REGION_MASK, y, p.z & REGION_MASK);
        int offset = RegionIndex(local);

        assert(offset >= 0 && offset < REGION_SIZE_3);
        
        List<uint16_t>& chunkData = region->chunks[offset];

        if (chunkData.size == 0)
        {
            complete = false;
            continue;
        }

        Chunk* chunk = group->chunks + y;
        chunk->state = CHUNK_LOADED_DATA;

        int i = 0; 
        int loc = 2;

        // If every block in the chunk is the same, the saved count will be 65536 
        // but wrap to 0 as uint16_t's max is 65535. If we read in a 0, 
        // interpret it to be that every block in this chunk is the same.
        if (chunkData[loc] == 0)
            FillChunk(chunk, chunkData[loc + 1]);
        else
        {
            while (i < CHUNK_SIZE_3)
            {
                int count = chunkData[loc++];
                Block block = chunkData[loc++];

                for (int j = 0; j < count; j++)
                    chunk->blocks[i++] = block;
            }
        }
    }

    return complete;
}

static void RemoveFromRegion(World* world, ChunkGroup* group)
{
    RegionP regionP = ChunkToRegionP(group->pos);

    EnterCriticalSection(&world->regionCS);

    RegionEntry it = GetRegion(world, regionP);

    Region* region = *it;
    region->activeCount--;

    if (region->activeCount == 0)
    {
        SaveRegion(world, region);
        memset(region, 0, sizeof(Region));
        world->regionPool.Return(region);
        world->regions.erase(it);
    }

    LeaveCriticalSection(&world->regionCS);
}

static void SaveGroup(GameState*, World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;
    ChunkP p = group->pos;

    for (int y = 0; y < WORLD_CHUNK_HEIGHT; y++)
    {
        Chunk* chunk = group->chunks + y;

        if (!chunk->modified)
            continue;

        RegionP regionP = ChunkToRegionP(p);
        
        RegionEntry entry = GetRegion(world, regionP);
        assert(entry != world->regions.end());
        Region* region = *entry;

        region->modified = true;

        ivec3 local = ivec3(p.x & REGION_MASK, y, p.z & REGION_MASK);
        int offset = RegionIndex(local);

        assert(offset >= 0 && offset < REGION_SIZE_3);

        List<uint16_t>& store = region->chunks[offset];
        region->hasData = true;
        store.Clear();
        store.Reserve(4096);

        store.Add((uint16_t)offset);

        // Holds the number of elements we will write. We won't know this value until 
        // after the RLE compression, but we should reserve its position in the data.
        store.Add(0);

        Block currentBlock = chunk->blocks[0];
        uint16_t count = 1;

        for (int i = 1; i < CHUNK_SIZE_3; i++)
        {
            Block block = chunk->blocks[i];

            if (block != currentBlock)
            {
                store.Add(count);
                store.Add(currentBlock);
                count = 1;
                currentBlock = block;
            }
            else count++;

            if (i == CHUNK_SIZE_3 - 1)
            {
                store.Add(count);
                store.Add(currentBlock);
            }
        }

        store[1] = (uint16_t)(store.size - 2);
        chunk->modified = false;
    }
}

static void SaveWorld(GameState* state, World* world)
{
    char path[MAX_PATH];
    sprintf(path, "%s\\WorldData.txt", world->savePath);

    WriteBinary(path, (char*)&world->properties, sizeof(WorldProperties));

    for (int i = 0; i < world->totalGroups; i++)
        SaveGroup(state, world, world->groups[i]);

    SaveAllRegions(world);
}

static bool LoadWorldFileData(GameState* state, World* world)
{
    world->savePath = new char[MAX_PATH]();
    world->savePath = strcat(strcat(world->savePath, state->savePath), "\\World");

    CreateDirectory(world->savePath, NULL);

    char path[MAX_PATH];
    sprintf(path, "%s\\WorldData.txt", world->savePath);

    if (PathFileExists(path))
    {
        ReadBinary(path, (char*)&world->properties);
        return true;
    }

    return false;
}
