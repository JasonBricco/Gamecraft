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
    Region* region = CallocStruct(Region);
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

        SerializedChunk* chunk = region->chunks + position;

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

                SerializedChunk chunk = region->chunks[index];

                if (chunk.size > 0)
                {
                    DWORD bytesToWrite = sizeof(uint16_t) * chunk.size;
                    DWORD bytesWritten;

                    if (!WriteFile(file, chunk.data, bytesToWrite, &bytesWritten, NULL))
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
    assert(world->firstRegion != nullptr);

    for (Region* region = world->firstRegion; region != nullptr; region = region->next)
        SaveRegion(world, region);
}

static inline void AddRegion(World* world, Region* region)
{
    if (world->firstRegion == nullptr)
        world->firstRegion = region;
    else
    {
        world->firstRegion->prev = region;
        region->next = world->firstRegion;
        world->firstRegion = region;
    }

    world->regionCount++;
}

static inline void RemoveRegion(World* world, Region* region)
{
    if (region->prev != nullptr)
        region->prev->next = region->next;

    if (region->next != nullptr)
        region->next->prev = region->prev;

    world->regionCount--;

    if (region == world->firstRegion)
        world->firstRegion = region->next;

    Free(region);
}

static void DeleteRegions(World* world)
{
    while (world->firstRegion != nullptr)
        RemoveRegion(world, world->firstRegion);
}

static void UnloadDistantRegions(World* world)
{
    Region* region = world->firstRegion;

    while (region != nullptr)
    {
        RegionP p = region->pos;
        ivec3 diff = p - world->playerRegion;

        Region* next = region->next;

        if (abs(diff.x) > 1 || abs(diff.z) > 1)
        {
            SaveRegion(world, region);
            RemoveRegion(world, region);
        }
        
        region = next;
    }
}

static Region* GetRegion(World* world, RegionP pos)
{
    for (Region* region = world->firstRegion; region != nullptr; region = region->next)
    {
        if (region->pos == pos)
            return region;
    }
        
    return nullptr;
}

static Region* GetOrLoadRegion(World* world, RegionP pos)
{
    Region* region = GetRegion(world, pos);

    if (region == nullptr)
    {
        region = LoadRegionFile(world, pos);
        AddRegion(world, region);

        if (world->regionCount > MAX_REGIONS)
            UnloadDistantRegions(world);
    }

    return region;
}

static bool LoadGroupFromDisk(World* world, ChunkGroup* group)
{
    ChunkP p = group->pos;
    RegionP regionP = ChunkToRegionP(p);

    EnterCriticalSection(&world->regionCS);
    Region* region = GetOrLoadRegion(world, regionP);
    LeaveCriticalSection(&world->regionCS);

    bool complete = true;

    for (int y = 0; y < WORLD_CHUNK_HEIGHT; y++)
    {
        ivec3 local = ivec3(p.x & REGION_MASK, y, p.z & REGION_MASK);
        int offset = RegionIndex(local);

        assert(offset >= 0 && offset < REGION_SIZE_3);
        
        SerializedChunk chunkData = region->chunks[offset];

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
        if (chunkData.data[loc] == 0)
            FillChunk(chunk, chunkData.data[loc + 1]);
        else
        {
            while (i < CHUNK_SIZE_3)
            {
                int count = chunkData.data[loc++];
                Block block = chunkData.data[loc++];

                for (int j = 0; j < count; j++)
                    chunk->blocks[i++] = block;
            }
        }
    }

    return complete;
}

static void SaveGroup(World* world, void* groupPtr)
{
    ChunkGroup* group = (ChunkGroup*)groupPtr;
    ChunkP p = group->pos;

    for (int y = 0; y < WORLD_CHUNK_HEIGHT; y++)
    {
        Chunk* chunk = group->chunks + y;

        if (!chunk->modified)
            continue;

        RegionP regionP = ChunkToRegionP(p);

        Region* region = GetRegion(world, regionP);
        assert(region != nullptr);

        region->modified = true;

        ivec3 local = ivec3(p.x & REGION_MASK, y, p.z & REGION_MASK);
        int offset = RegionIndex(local);

        assert(offset >= 0 && offset < REGION_SIZE_3);

        SerializedChunk& store = region->chunks[offset];
        region->hasData = true;
        store.Clear();

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

        store.data[1] = (uint16_t)(store.size - 2);
        chunk->modified = false;
    }
}

static void SaveWorld(World* world)
{
    char path[MAX_PATH];
    sprintf(path, "%s\\WorldData.txt", world->savePath);

    WriteBinary(path, (char*)&world->properties, sizeof(WorldProperties));

    for (int i = 0; i < world->totalGroups; i++)
        SaveGroup(world, world->groups[i]);

    SaveAllRegions(world);
}

static bool LoadWorldFileData(GameState* state, World* world)
{
    world->savePath = (char*)calloc(1, MAX_PATH * sizeof(char));
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
