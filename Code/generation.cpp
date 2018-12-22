//
// Jason Bricco
//

static inline float GetNoiseValue2D(float* noiseSet, int x, int z)
{
    return (noiseSet[x * (CHUNK_SIZE_X) + z] + 1.0f) / 2.0f;
}

static inline float GetRawNoiseValue2D(float* noiseSet, int x, int z)
{
    return noiseSet[x * (CHUNK_SIZE_X) + z];
}

static inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z, int maxY)
{
    return (noiseSet[z + CHUNK_SIZE_X * (y + maxY * x)] + 1.0f) / 2.0f;
}

static inline float* GetNoise2D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = CHUNK_SIZE_X;
    int sizeY = 1;
    int sizeZ = CHUNK_SIZE_X;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static inline float* GetNoise3D(Noise* noise, Noise::NoiseType type, int x, int z, int maxY, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = CHUNK_SIZE_X;
    int sizeZ = CHUNK_SIZE_X;
    return noise->GetNoiseSet(x, 0, z, sizeX, maxY, sizeZ, scale);
}

static void GenerateChunkTerrain(World* world, Chunk* chunk)
{
    WorldPos start = chunk->cPos * CHUNK_SIZE_X;

    Noise* noise = Noise::NewFastNoiseSIMD();
    noise->SetSeed(world->seed);

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

    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_X; z++)
        {
            // Determine if this column is inside of the island. The world center is assumed to be the origin (0, 0).
            int valueInCircle = Square(start.x + x) + Square(start.z + z);

            if (valueInCircle < world->sqRadius)
            {
                float p = 1.0f;

                if (valueInCircle > world->falloffRadius)
                    p = 1.0f - ((valueInCircle - world->falloffRadius) / (float)(world->sqRadius - world->falloffRadius));

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
                surfaceMap[z * CHUNK_SIZE_X + x] = height;

                maxY = Max(maxY, Max(height, SEA_LEVEL));
            }
            else 
            {
                surfaceMap[z * CHUNK_SIZE_X + x] = 0;
                maxY = Max(maxY, SEA_LEVEL);
            }
        }
    }

    noise->SetFractalOctaves(2);
    noise->SetFrequency(0.015f);
    noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, Noise::SimplexFractal, start.x, start.z, maxY, 0.2f);

    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_X; z++)
        {
            int height = surfaceMap[z * CHUNK_SIZE_X + x];

            for (int y = 0; y <= maxY; y++)
            {
                float compVal = GetNoiseValue3D(comp, x, y, z, maxY);

                if (y <= height - 10)
                {
                    if (compVal <= 0.2f)
                    {
                        SetBlock(chunk, x, y, z, BLOCK_STONE);
                        continue;
                    }
                }

                if (y == height)
                    SetBlock(chunk, x, y, z, BLOCK_GRASS);
                else if (y > height && y <= SEA_LEVEL)
                    SetBlock(chunk, x, y, z, BLOCK_WATER);
                else 
                {
                    if (y < height)
                        SetBlock(chunk, x, y, z, BLOCK_DIRT);
                }
            }
        }
    }

    Noise::FreeNoiseSet(ridged);
    Noise::FreeNoiseSet(base);
    Noise::FreeNoiseSet(biome);
    Noise::FreeNoiseSet(comp);

    delete noise;
}
