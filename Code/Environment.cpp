//
// Gamecraft
//

static void FadeAmbient(World* world, Renderer& rend, float target, float time)
{
    Biome& biome = GetCurrentBiome(world);

    LerpData<vec3>& col = rend.clearColorLerp;
    col.start = rend.clearColor;
    col.end = biome.skyColor * target;
    col.t = 0.0f;
    col.time = time;

    LerpData<float>& amb = rend.ambientLerp;
    amb.start = rend.ambient;
    amb.end = target;
    amb.t = 0.0f;
    amb.time = time;

    rend.fadeEnv = true;
}

static void UpdateEnvironment(GameState* state, World* world, float deltaTime)
{
    Biome& biome = GetCurrentBiome(world);
    ParticleEmitter& emitter = *biome.weather.emitter;

    Player* player = world->player;
    emitter.pos = vec3(player->pos.x, Max(265.0f, player->pos.y + 10.0f), player->pos.z);
    emitter.update(emitter, world, deltaTime);

    Renderer& rend = state->renderer;

    if (rend.fadeEnv)
    {
        bool complete = false;
        rend.clearColor = Lerp(rend.clearColorLerp, deltaTime, complete);
        rend.ambient = Lerp(rend.ambientLerp, deltaTime, complete);

        ApplyClearColor(state, rend);

        if (complete)
            rend.fadeEnv = false;
    }
}

static void ResetEnvironment(GameState* state, World* world)
{
    Biome& biome = GetCurrentBiome(world);

    ParticleEmitter& emitter = *biome.weather.emitter;

    emitter.active = false;
    DestroyAllParticles(emitter);

    Renderer& rend = state->renderer;
    rend.ambient = 1.0f;
    rend.clearColor = biome.skyColor;

    ApplyClearColor(state, rend);
}

static void CreateBiomes(GameState* state, World* world)
{
	InitParticleEmitter(state->renderer, state->rain, 1, 8, 8000);
	SetEmitterProperties(state->rain, IMAGE_RAIN, 12, 20.0f, 10.0f, UpdateRainParticles);

	InitParticleEmitter(state->renderer, state->snow, 4, 4, 12000);
	SetEmitterProperties(state->snow, IMAGE_SNOWFLAKE, 8, 20.0f, 20.0f, UpdateSnowParticles);

    Biome& grassy = world->biomes[BIOME_GRASSY];
    grassy.name = "Grassy";
    grassy.type = BIOME_GRASSY;
    grassy.func = GenerateGrassyTerrain;
    grassy.skyColor = vec3(0.53f, 0.80f, 0.92f);
    grassy.weather.emitter = &state->rain;

    Biome& snow = world->biomes[BIOME_SNOW];
    snow.name = "Snow";
    snow.type = BIOME_SNOW;
    snow.func = GenerateSnowTerrain;
    snow.skyColor = vec3(0.53f, 0.80f, 0.92f);
    snow.weather.emitter = &state->snow;

    Biome& desert = world->biomes[BIOME_DESERT];
    desert.name = "Desert";
    desert.type = BIOME_DESERT;
    desert.func = GenerateDesertTerrain;
    desert.skyColor = vec3(0.53f, 0.80f, 0.92f);
    desert.weather.emitter = &state->rain;

    Biome& grid = world->biomes[BIOME_GRID];
    grid.name = "Grid";
    grid.type = BIOME_GRID;
    grid.func = GenerateGridTerrain;
    grid.skyColor = vec3(0.53f, 0.80f, 0.92f);
    grid.weather.emitter = &state->rain;

    Biome& flat = world->biomes[BIOME_FLAT];
    flat.name = "Flat";
    flat.type = BIOME_FLAT;
    flat.func = GenerateFlatTerrain;
    flat.skyColor = vec3(0.53f, 0.80f, 0.92f);
    flat.weather.emitter = &state->rain;

    Biome& empty = world->biomes[BIOME_VOID];
    empty.name = "Void";
    empty.type = BIOME_VOID;
    empty.func = GenerateVoidTerrain;
    empty.skyColor = vec3(0.0f);
    empty.weather.emitter = &state->rain;
}
