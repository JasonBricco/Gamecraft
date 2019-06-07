//
// Gamecraft
//

static inline float GetNoiseValue2D(float* noiseSet, int x, int z)
{
    return (noiseSet[x * (CHUNK_SIZE_H) + z] + 1.0f) / 2.0f;
}

static inline float GetRawNoiseValue2D(float* noiseSet, int x, int z)
{
    return noiseSet[x * (CHUNK_SIZE_H) + z];
}

static inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z, int maxY)
{
    return (noiseSet[z + CHUNK_SIZE_H * (y + maxY * x)] + 1.0f) / 2.0f;
}

static inline float* GetNoise2D(Noise* noise, int x, int y, int z, float scale = 1.0f)
{
    int sizeX = CHUNK_SIZE_H;
    int sizeY = 1;
    int sizeZ = CHUNK_SIZE_H;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static inline float* GetNoise3D(Noise* noise, int x, int z, int maxY, float scale = 1.0f)
{
    int sizeX = CHUNK_SIZE_H;
    int sizeZ = CHUNK_SIZE_H;
    return noise->GetNoiseSet(x, 0, z, sizeX, maxY, sizeZ, scale);
}

static inline void CreateBox(ChunkGroup* group, ivec3 min, ivec3 max, Block block)
{
    for (int z = min.z; z <= max.z; z++)
    {
        for (int y = min.y; y <= max.y; y++)
        {
            for (int x = min.x; x <= max.x; x++)
            {
                assert(BlockInsideGroup(x, y, z));
                SetBlock(group, x, y, z, block);
            }
        }
    }
}

static inline void CreateTree(ChunkGroup* group, ivec3 base, int minHeight, int maxHeight, Block wood, Block leaves)
{
    int height = RandRange(minHeight, maxHeight);

    for (int j = 0; j < height; j++)
        SetBlock(group, base.x, base.y + j, base.z, wood);

    int startY = base.y + height;

    CreateBox(group, ivec3(base.x - 1, startY, base.z - 1), ivec3(base.x + 1, startY, base.z + 1), leaves);
    CreateBox(group, ivec3(base.x - 2, startY + 1, base.z - 1), ivec3(base.x + 2, startY + 2, base.z + 1), leaves);
    CreateBox(group, ivec3(base.x - 1, startY + 1, base.z + 2), ivec3(base.x + 1, startY + 2, base.z + 2), leaves);
    CreateBox(group, ivec3(base.x - 1, startY + 1, base.z - 2), ivec3(base.x + 1, startY + 2, base.z - 2), leaves);
    CreateBox(group, ivec3(base.x - 1, startY + 3, base.z - 1), ivec3(base.x + 1, startY + 3, base.z + 1), leaves);
}

static inline bool IsWithinIsland(World* world, WorldP start, int x, int z, float& p)
{
    int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

    if (valueInCircle < world->properties.radius)
    {
        p = 1.0f;

        if (valueInCircle > world->falloffRadius)
            p = 1.0f - ((valueInCircle - world->falloffRadius) / (float)(world->properties.radius - world->falloffRadius));

        return true;
    }

    return false;
}

static void GenerateForestTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.015f);
    noise->SetFractalOctaves(4);
    noise->SetFractalType(Noise::RigidMulti);
    float* ridged = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.025f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    noise->SetNoiseType(Noise::Simplex);
    noise->SetFrequency(0.01f);
    float* biome = GetNoise2D(noise, start.x, 0, start.z);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    int seaLevel = 10;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            float p;

            if (IsWithinIsland(world, start, x, z, p))
            {
                float terrainVal;
                float biomeVal = GetRawNoiseValue2D(biome, x, z);

                float flat = GetNoiseValue2D(base, x, z);
                flat = ((flat * 0.2f) * 30.0f) + 10.0f;

                float mountain = GetNoiseValue2D(ridged, x, z);
                mountain = (pow(mountain, 3.5f) * 30.0f) + 10.0f;

                float lower = 0.0f;
                float upper = 0.6f;

                if (biomeVal < lower)
                    terrainVal = flat;
                else if (biomeVal > upper)
                    terrainVal = mountain;
                else
                {
                    // If we're close to the boundary between the two terrain types,
                    // interpolate between them for a smooth transition.
                    float a = SCurve3((biomeVal - lower) / (upper - lower));
                    terrainVal = Lerp(flat, mountain, a);
                }

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, seaLevel));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, seaLevel);
            }
        }
    }

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, start.x, start.z, maxY + 1, 0.2f);

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        LWorldP lwP = chunk->lwPos;

        if (lwP.y > maxY || chunk->state == CHUNK_LOADED_DATA)
            continue;

        int limY = Min(lwP.y + CHUNK_V_MASK, maxY);

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int wY = lwP.y; wY <= limY; wY++)
            {
                int y = wY & CHUNK_V_MASK;

                for (int x = 0; x < CHUNK_SIZE_H; x++)
                {
                    int height = surfaceMap[z * CHUNK_SIZE_H + x];
                    float compVal = GetNoiseValue3D(comp, x, wY, z, maxY);

                    if (wY <= height - 4)
                    {
                        if (compVal <= 0.2f)
                        {
                            SetBlock(chunk, x, y, z, BLOCK_STONE);
                            continue;
                        }
                    }

                    if (wY == height)
                        SetBlock(chunk, x, y, z, BLOCK_GRASS);
                    else if (wY > height && wY <= seaLevel)
                        SetBlock(chunk, x, y, z, BLOCK_WATER);
                    else 
                    {
                        if (wY < height)
                            SetBlock(chunk, x, y, z, BLOCK_DIRT);
                    }
                }
            }
        }
    }

    int treeNum = RandRange(3, 6);

    for (int i = 0; i < treeNum; i++)
    {
        int rX = RandRange(3, CHUNK_SIZE_H - 4);
        int rZ = RandRange(3, CHUNK_SIZE_H - 4);

        int surface = surfaceMap[rZ * CHUNK_SIZE_H + rX];

        if (surface > seaLevel)
            CreateTree(group, ivec3(rX, surface + 1, rZ), 3, 5, BLOCK_WOOD, BLOCK_LEAVES);
    }

    Noise::FreeNoiseSet(comp);
    Noise::FreeNoiseSet(ridged);
    Noise::FreeNoiseSet(base);
    Noise::FreeNoiseSet(biome);

    delete noise;
}

static void GenerateIslandsTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.015f);
    noise->SetFractalOctaves(4);
    noise->SetFractalType(Noise::RigidMulti);
    float* ridged = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.025f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    noise->SetNoiseType(Noise::Simplex);
    noise->SetFrequency(0.01f);
    float* biome = GetNoise2D(noise, start.x, 0, start.z);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    int seaLevel = 20;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            float p;

            if (IsWithinIsland(world, start, x, z, p))
            {
                float terrainVal;
                float biomeVal = GetRawNoiseValue2D(biome, x, z);

                float flat = GetNoiseValue2D(base, x, z);
                flat = ((flat * 0.2f) * 30.0f) + 10.0f;

                float mountain = GetNoiseValue2D(ridged, x, z);
                mountain = (pow(mountain, 3.5f) * 60.0f) + 20.0f;

                float lower = 0.0f;
                float upper = 0.6f;

                if (biomeVal < lower)
                    terrainVal = flat;
                else if (biomeVal > upper)
                    terrainVal = mountain;
                else
                {
                    float a = SCurve3((biomeVal - lower) / (upper - lower));
                    terrainVal = Lerp(flat, mountain, a);
                }

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, seaLevel));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, seaLevel);
            }
        }
    }

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, start.x, start.z, maxY + 1, 0.2f);

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        LWorldP lwP = chunk->lwPos;

        if (lwP.y > maxY || chunk->state == CHUNK_LOADED_DATA)
            continue;

        int limY = Min(lwP.y + CHUNK_V_MASK, maxY);

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int wY = lwP.y; wY <= limY; wY++)
            {
                int y = wY & CHUNK_V_MASK;

                for (int x = 0; x < CHUNK_SIZE_H; x++)
                {
                    int height = surfaceMap[z * CHUNK_SIZE_H + x];
                    float compVal = GetNoiseValue3D(comp, x, wY, z, maxY);

                    if (wY <= height - 4)
                    {
                        if (compVal <= 0.2f)
                        {
                            SetBlock(chunk, x, y, z, BLOCK_STONE);
                            continue;
                        }
                    }

                    if (wY == height)
                        SetBlock(chunk, x, y, z, BLOCK_GRASS);
                    else if (wY > height && wY <= seaLevel)
                        SetBlock(chunk, x, y, z, BLOCK_WATER);
                    else 
                    {
                        if (wY < height)
                            SetBlock(chunk, x, y, z, BLOCK_DIRT);
                    }
                }
            }
        }
    }

    int treeNum = RandRange(1, 3);

    for (int i = 0; i < treeNum; i++)
    {
        int rX = RandRange(3, CHUNK_SIZE_H - 4);
        int rZ = RandRange(3, CHUNK_SIZE_H - 4);

        int surface = surfaceMap[rZ * CHUNK_SIZE_H + rX];

        if (surface > seaLevel)
            CreateTree(group, ivec3(rX, surface + 1, rZ), 3, 5, BLOCK_WOOD, BLOCK_LEAVES);
    }

    Noise::FreeNoiseSet(comp);
    Noise::FreeNoiseSet(ridged);
    Noise::FreeNoiseSet(base);
    Noise::FreeNoiseSet(biome);

    delete noise;
}

static void GenerateGridTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;

        if (chunk->state == CHUNK_LOADED_DATA)
            continue;
        
        for (int z = 0; z < CHUNK_SIZE_H; z += 2)
        {
            for (int x = 0; x < CHUNK_SIZE_H; x += 2)
            {
                int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

                if (valueInCircle < world->properties.radius)
                {
                    for (int y = 2; y < 14; y += 2)
                        SetBlock(chunk, x, y, z, BLOCK_METAL_CRATE);
                }
            }
        }

        for (int z = -2; z <= 2; z += 2)
        {
            for (int x = -2; x <= 2; x += 2)
            {
                for (int y = 14; y < 40; y += 2)
                    SetBlock(chunk, x + 16, y, z + 16, BLOCK_METAL_CRATE);
            }
        }

        for (int y = 40; y <= 64; y += 2)
            SetBlock(chunk, 16, y, 16, BLOCK_METAL_CRATE);
    }
}

static void GenerateSnowTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFractalType(Noise::Billow);
    noise->SetFrequency(0.025f);
    float* base = GetNoise2D(noise, start.x, 0, start.z, 0.25f);

    noise->SetNoiseType(Noise::PerlinFractal);
    noise->SetFractalOctaves(3);
    noise->SetFractalType(Noise::FBM);
    noise->SetFrequency(0.01f);
    float* ice = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    int seaLevel = 16;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            float p;

            if (IsWithinIsland(world, start, x, z, p))
            {
                float terrainVal = GetNoiseValue2D(base, x, z);
                terrainVal = ((terrainVal * 0.2f) * 60.0f) + 12.0f;

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, seaLevel));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, seaLevel);
            }
        }
    }

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, start.x, start.z, maxY + 1, 0.2f);

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        LWorldP lwP = chunk->lwPos;

        if (lwP.y > maxY || chunk->state == CHUNK_LOADED_DATA)
            continue;

        int limY = Min(lwP.y + CHUNK_V_MASK, maxY);

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int wY = lwP.y; wY <= limY; wY++)
            {
                int y = wY & CHUNK_V_MASK;

                for (int x = 0; x < CHUNK_SIZE_H; x++)
                {
                    int height = surfaceMap[z * CHUNK_SIZE_H + x];
                    float compVal = GetNoiseValue3D(comp, x, wY, z, maxY);

                    if (wY <= height - 4)
                    {
                        if (compVal <= 0.2f)
                        {
                            SetBlock(chunk, x, y, z, BLOCK_STONE);
                            continue;
                        }
                    }

                    if (wY == height)
                        SetBlock(chunk, x, y, z, BLOCK_SNOW);
                    else if (wY > height && wY < seaLevel)
                        SetBlock(chunk, x, y, z, BLOCK_WATER);
                    else if (wY == seaLevel)
                    {
                        if (GetNoiseValue2D(ice, x, z) > 0.5f)
                            SetBlock(chunk, x, y, z, BLOCK_ICE);
                        else SetBlock(chunk, x, y, z, BLOCK_WATER);
                    }
                    else 
                    {
                        if (wY < height)
                            SetBlock(chunk, x, y, z, BLOCK_DIRT);
                    }
                }
            }
        }
    }

    Noise::FreeNoiseSet(comp);
    Noise::FreeNoiseSet(base);

    delete noise;
}

static void GenerateDesertTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.05f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, start.x, 0, start.z, 0.25f);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    int seaLevel = 10;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            float p;

            if (IsWithinIsland(world, start, x, z, p))
            {
                float terrainVal = GetNoiseValue2D(base, x, z);
                terrainVal = ((terrainVal * 0.2f) * 30.0f) + 20.0f;

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, seaLevel));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, seaLevel);
            }
        }
    }

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        LWorldP lwP = chunk->lwPos;

        if (lwP.y > maxY || chunk->state == CHUNK_LOADED_DATA)
            continue;

        int limY = Min(lwP.y + CHUNK_V_MASK, maxY);

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int wY = lwP.y; wY <= limY; wY++)
            {
                int y = wY & CHUNK_V_MASK;

                for (int x = 0; x < CHUNK_SIZE_H; x++)
                {
                    int height = surfaceMap[z * CHUNK_SIZE_H + x];

                    if (wY <= height)
                        SetBlock(chunk, x, y, z, BLOCK_SAND);
                    else if (wY > height && wY <= seaLevel)
                        SetBlock(chunk, x, y, z, BLOCK_WATER);
                }
            }
        }
    }
    
    int cactusNum = RandRange(0, 2);

    for (int i = 0; i < cactusNum; i++)
    {
        int rX = RandRange(0, CHUNK_SIZE_H - 1);
        int rZ = RandRange(0, CHUNK_SIZE_H - 1);

        int height = RandRange(2, 4);

        for (int j = 1; j <= height; j++)
            SetBlock(group, rX, surfaceMap[rZ * CHUNK_SIZE_H + rX] + j, rZ, BLOCK_CACTUS);
    }

    Noise::FreeNoiseSet(base);
    delete noise;
}

static void GenerateVolcanicTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.015f);
    noise->SetFractalOctaves(4);
    noise->SetFractalType(Noise::RigidMulti);
    float* ridged = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFrequency(0.025f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, start.x, 0, start.z, 0.5f);

    noise->SetNoiseType(Noise::Simplex);
    noise->SetFrequency(0.005f);
    float* biome = GetNoise2D(noise, start.x, 0, start.z);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    int seaLevel = 12;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            float p;

            if (IsWithinIsland(world, start, x, z, p))
            {
                float terrainVal;
                float biomeVal = GetRawNoiseValue2D(biome, x, z);

                float flat = GetNoiseValue2D(base, x, z);
                flat = ((flat * 0.2f) * 30.0f) + 10.0f;

                float mountain = GetNoiseValue2D(ridged, x, z);
                mountain = (pow(mountain, 3.5f) * 60.0f) + 20.0f;

                float lower = 0.0f;
                float upper = 0.6f;

                if (biomeVal < lower)
                    terrainVal = flat;
                else if (biomeVal > upper)
                    terrainVal = mountain;
                else
                {
                    float a = SCurve3((biomeVal - lower) / (upper - lower));
                    terrainVal = Lerp(flat, mountain, a);
                }

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, seaLevel));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, seaLevel);
            }
        }
    }

    noise->SetNoiseType(Noise::SimplexFractal);
    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, start.x, start.z, maxY + 1, 0.2f);

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        LWorldP lwP = chunk->lwPos;

        if (lwP.y > maxY || chunk->state == CHUNK_LOADED_DATA)
            continue;

        int limY = Min(lwP.y + CHUNK_V_MASK, maxY);

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int wY = lwP.y; wY <= limY; wY++)
            {
                int y = wY & CHUNK_V_MASK;

                for (int x = 0; x < CHUNK_SIZE_H; x++)
                {
                    int height = surfaceMap[z * CHUNK_SIZE_H + x];
                    float compVal = GetNoiseValue3D(comp, x, wY, z, maxY);

                    if (wY <= height - 4)
                    {
                        if (compVal <= 0.2f)
                        {
                            SetBlock(chunk, x, y, z, BLOCK_STONE);
                            continue;
                        }
                    }

                    if (wY <= height)
                        SetBlock(chunk, x, y, z, BLOCK_OBSIDIAN);
                    else if (wY > height && wY <= seaLevel)
                        SetBlock(chunk, x, y, z, BLOCK_LAVA);
                }
            }
        }
    }

    Noise::FreeNoiseSet(comp);
    Noise::FreeNoiseSet(ridged);
    Noise::FreeNoiseSet(base);
    Noise::FreeNoiseSet(biome);

    delete noise;
}

static void GenerateFlatTerrain(World* world, ChunkGroup* group)
{
    TIMED_FUNCTION;

    WorldP start = ChunkToWorldP(group->pos);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    int seaLevel = 12;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            float p;

            if (IsWithinIsland(world, start, x, z, p))
            {
                int height = (int)(seaLevel * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;
                maxY = Max(maxY, Max(height, seaLevel));
            }
            else
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, seaLevel);
            }
        }
    }

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;
        LWorldP lwP = chunk->lwPos;

        if (lwP.y > maxY || chunk->state == CHUNK_LOADED_DATA)
            continue;

        int limY = Min(lwP.y + CHUNK_V_MASK, maxY);

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int wY = lwP.y; wY <= limY; wY++)
            {
                int y = wY & CHUNK_V_MASK;

                for (int x = 0; x < CHUNK_SIZE_H; x++)
                {
                    int height = surfaceMap[z * CHUNK_SIZE_H + x];

                    if (wY == height)
                        SetBlock(chunk, x, y, z, BLOCK_GRASS);
                    else if (wY > height && wY <= seaLevel)
                        SetBlock(chunk, x, y, z, BLOCK_WATER);
                    else 
                    {
                        if (wY < height)
                            SetBlock(chunk, x, y, z, BLOCK_DIRT);
                    }
                }
            }
        }
    }
}

static void GenerateVoidTerrain(World*, ChunkGroup* group)
{
    TIMED_FUNCTION;
    
    if (group->pos == ivec3(0))
    {
        Chunk* chunk = group->chunks;

        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            for (int x = 0; x < CHUNK_SIZE_H; x++)
                SetBlock(chunk, x, 40, z, BLOCK_GRASS);
        }
    }
}
