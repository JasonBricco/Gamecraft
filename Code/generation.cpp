//
// Jason Bricco
//

static inline float GetNoiseValue2D(float* noiseSet, int x, int z)
{
	return (noiseSet[x * (PADDED_CHUNK_SIZE) + z] + 1.0f) / 2.0f;
}

static inline float GetRawNoiseValue2D(float* noiseSet, int x, int z)
{
    return noiseSet[x * (PADDED_CHUNK_SIZE) + z];
}

static inline float GetNoiseValue3D(float* noiseSet, int x, int y, int z)
{
	return (noiseSet[z + PADDED_CHUNK_SIZE * (y + PADDED_CHUNK_SIZE * x)] + 1.0f) / 2.0f;
}

static inline float* GetNoise2D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = PADDED_CHUNK_SIZE;
    int sizeY = 1;
    int sizeZ = PADDED_CHUNK_SIZE;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static inline float* GetNoise3D(Noise* noise, Noise::NoiseType type, int x, int y, int z, float scale = 1.0f)
{
    noise->SetNoiseType(type);
    int sizeX = PADDED_CHUNK_SIZE;
    int sizeY = PADDED_CHUNK_SIZE;
    int sizeZ = PADDED_CHUNK_SIZE;
    return noise->GetNoiseSet(x, y, z, sizeX, sizeY, sizeZ, scale);
}

static void GenerateChunkTerrain(World* world, Chunk* chunk)
{
    WorldPos start = chunk->cPos * CHUNK_SIZE;
    start -= 1;

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

	for (int x = 0; x < PADDED_CHUNK_SIZE; x++)
    {
        for (int z = 0; z < PADDED_CHUNK_SIZE; z++)
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

        	for (int y = 0; y < PADDED_CHUNK_SIZE; y++)
        	{
        		int wY = start.y + y;
        		float compVal = GetNoiseValue3D(comp, x, y, z);

        		if (wY <= height - 10)
        		{
	        		if (compVal <= 0.2f)
	        		{
	        			SetBlockPadded(chunk, x, y, z, BLOCK_STONE);
	        			continue;
	        		}
	        	}

        		if (wY == height)
        			SetBlockPadded(chunk, x, y, z, BLOCK_GRASS);
        		else if (wY > height && wY <= SEA_LEVEL)
        			SetBlockPadded(chunk, x, y, z, BLOCK_WATER);
        		else 
                {
                    if (wY < height)
                        SetBlockPadded(chunk, x, y, z, BLOCK_DIRT);
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
