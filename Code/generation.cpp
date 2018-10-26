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

static inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z)
{
	return (noiseSet[z + CHUNK_SIZE_X * (y + CHUNK_SIZE_Y * x)] + 1.0f) / 2.0f;
}

static inline float* GetNoise2D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = CHUNK_SIZE_X;
    int sizeY = 1;
    int sizeZ = CHUNK_SIZE_X;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static inline float* GetNoise3D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = CHUNK_SIZE_X;
    int sizeY = CHUNK_SIZE_Y;
    int sizeZ = CHUNK_SIZE_X;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
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

	noise->SetFractalOctaves(2);
	noise->SetFrequency(0.015f);
	noise->SetFractalType(Noise::FBM);
    float* comp = GetNoise3D(noise, Noise::SimplexFractal, start.x, start.y, start.z, 0.2f);

	for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_X; z++)
        {
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

        	int height = (int)terrainVal;

            int limY = Max(height, SEA_LEVEL);
        	for (int y = 0; y <= limY; y++)
        	{
        		float compVal = GetNoiseValue3D(comp, x, y, z);

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
