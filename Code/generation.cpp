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

static inline float* GetNoise2D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = CHUNK_SIZE_H;
    int sizeY = 1;
    int sizeZ = CHUNK_SIZE_H;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static inline float* GetNoise3D(Noise* noise, Noise::NoiseType type, int x, int z, int maxY, float scale = 1.0f)
{
    noise->SetNoiseType(type);
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

static void GenerateGrassyTerrain(World* world, ChunkGroup* group)
{
    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetFrequency(0.015f);
    noise->SetFractalOctaves(4);
    noise->SetFractalType(Noise::RigidMulti);
    float* ridged = GetNoise2D(noise, Noise::SimplexFractal, start.x, 0, start.z, 0.5f);

    noise->SetFrequency(0.025f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, Noise::SimplexFractal, start.x, 0, start.z, 0.5f);

    noise->SetFrequency(0.01f);
    float* biome = GetNoise2D(noise, Noise::Simplex, start.x, 0, start.z);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            // Determine if this column is inside of the island. The world center is assumed to be the origin (0, 0).
            int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

            if (valueInCircle < world->properties.radius)
            {
                float p = 1.0f;

                if (valueInCircle > world->falloffRadius)
                    p = 1.0f - ((valueInCircle - world->falloffRadius) / (float)(world->properties.radius - world->falloffRadius));

                float terrainVal;
                float biomeVal = GetRawNoiseValue2D(biome, x, z);

                // Value for flat terrain.
                float flat = GetNoiseValue2D(base, x, z);
                flat = ((flat * 0.2f) * 30.0f) + 10.0f;

                // Value for mountainous terrain.
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
                    // If we're close to the boundary between the two terrain types,
                    // interpolate between them for a smooth transition.
                    float a = SCurve3((biomeVal - lower) / (upper - lower));
                    terrainVal = Lerp(flat, mountain, a);
                }

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, SEA_LEVEL));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, SEA_LEVEL);
            }
        }
    }

    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, Noise::SimplexFractal, start.x, start.z, maxY + 1, 0.2f);

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
                    else if (wY > height && wY <= SEA_LEVEL)
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

    // Generate trees.
    int treeNum = RandRange(1, 3);

    for (int i = 0; i < treeNum; i++)
    {
        int rX = RandRange(3, CHUNK_SIZE_H - 4);
        int rZ = RandRange(3, CHUNK_SIZE_H - 4);

        int surface = surfaceMap[rZ * CHUNK_SIZE_H + rX];

        if (surface > SEA_LEVEL)
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
    WorldP start = ChunkToWorldP(group->pos);

    for (int i = 0; i < WORLD_CHUNK_HEIGHT; i++)
    {
        Chunk* chunk = group->chunks + i;

        if (chunk->state == CHUNK_LOADED_DATA)
            continue;

        if (i == 0)
        {
            for (int x = 0; x < CHUNK_SIZE_H; x++)
            {
                for (int z = 0; z < CHUNK_SIZE_H; z++)
                {
                    for (int y = 0; y < SEA_LEVEL; y++)
                    {
                        if (GetBlock(chunk, x, y, z) == BLOCK_AIR)
                            SetBlock(chunk, x, y, z, BLOCK_WATER);
                    }
                }
            }
        }
        
        for (int x = 0; x < CHUNK_SIZE_H; x += 2)
        {
            for (int z = 0; z < CHUNK_SIZE_H; z += 2)
            {
                int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

                if (valueInCircle < world->properties.radius)
                {
                    for (int y = 0; y < CHUNK_SIZE_V; y += 2)
                        SetBlock(chunk, x, y, z, BLOCK_METAL_CRATE);
                }
            }
        }
    }
}

static void GenerateSnowTerrain(World* world, ChunkGroup* group)
{
    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetFrequency(0.025f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, Noise::SimplexFractal, start.x, 0, start.z, 0.25f);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

            if (valueInCircle < world->properties.radius)
            {
                float p = 1.0f;

                if (valueInCircle > world->falloffRadius)
                    p = 1.0f - ((valueInCircle - world->falloffRadius) / (float)(world->properties.radius - world->falloffRadius));

                float terrainVal = GetNoiseValue2D(base, x, z);
                terrainVal = ((terrainVal * 0.2f) * 30.0f) + 10.0f;

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, SEA_LEVEL));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, SEA_LEVEL);
            }
        }
    }

    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, Noise::SimplexFractal, start.x, start.z, maxY + 1, 0.2f);

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
                    else if (wY > height && wY <= SEA_LEVEL)
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

    Noise::FreeNoiseSet(comp);
    Noise::FreeNoiseSet(base);

    delete noise;
}

static void GenerateDesertTerrain(World* world, ChunkGroup* group)
{
    WorldP start = ChunkToWorldP(group->pos);

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->properties.seed);

    noise->SetFrequency(0.05f);
    noise->SetFractalType(Noise::Billow);
    float* base = GetNoise2D(noise, Noise::SimplexFractal, start.x, 0, start.z, 0.25f);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

            if (valueInCircle < world->properties.radius)
            {
                float p = 1.0f;

                if (valueInCircle > world->falloffRadius)
                    p = 1.0f - ((valueInCircle - world->falloffRadius) / (float)(world->properties.radius - world->falloffRadius));

                float terrainVal = GetNoiseValue2D(base, x, z);
                terrainVal = ((terrainVal * 0.2f) * 30.0f) + 20.0f;

                int height = (int)(terrainVal * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;

                maxY = Max(maxY, Max(height, SEA_LEVEL));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, SEA_LEVEL);
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
                    else if (wY > height && wY <= SEA_LEVEL)
                        SetBlock(chunk, x, y, z, BLOCK_WATER);
                }
            }
        }
    }

    // Generate cacti.
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

static void GenerateFlatTerrain(World* world, ChunkGroup* group)
{
    WorldP start = ChunkToWorldP(group->pos);

    int surfaceMap[CHUNK_SIZE_2];
    int maxY = 0;

    for (int x = 0; x < CHUNK_SIZE_H; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_H; z++)
        {
            int valueInCircle = (int)sqrt(Square(start.x + x) + Square(start.z + z));

            if (valueInCircle < world->properties.radius)
            {
                float p = 1.0f;

                if (valueInCircle > world->falloffRadius)
                    p = 1.0f - ((valueInCircle - world->falloffRadius) / (float)(world->properties.radius - world->falloffRadius));

                int height = (int)(SEA_LEVEL * p);
                surfaceMap[z * CHUNK_SIZE_H + x] = height;
                maxY = Max(maxY, Max(height, SEA_LEVEL));
            }
            else
            {
                surfaceMap[z * CHUNK_SIZE_H + x] = 0;
                maxY = Max(maxY, SEA_LEVEL);
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
                    else if (wY > height && wY <= SEA_LEVEL)
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
