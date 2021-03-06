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
    ParticleEmitter& rainEffect = state->rain;
	InitParticleEmitter(state->renderer, rainEffect, IMAGE_RAIN, 1, 8, 8000);
    rainEffect.spawnCount = 12;
    rainEffect.lifetime = 10.0f;
    rainEffect.update = UpdateRainParticles;

	ParticleEmitter& snowEffect = state->snow;
	InitParticleEmitter(state->renderer, snowEffect, IMAGE_SNOWFLAKE, 4, 4, 12000);
    snowEffect.spawnCount = 8;
    snowEffect.update = UpdateSnowParticles;

    ParticleEmitter& ashEffect = state->ash;
    InitParticleEmitter(state->renderer, ashEffect, IMAGE_ASH_PARTICLE, 4, 4, 10000);
    ashEffect.spawnCount = 4;
    ashEffect.lifetime = 30.0f;
    ashEffect.update = UpdateAshParticles;

    Biome& forest = world->biomes[BIOME_FOREST];
    forest.name = "Forest";
    forest.type = BIOME_FOREST;
    forest.func = GenerateForestTerrain;
    forest.skyColor = vec3(0.53f, 0.80f, 0.92f);
    forest.weather.emitter = &state->rain;

    Biome& islands = world->biomes[BIOME_ISLANDS];
    islands.name = "Islands";
    islands.type = BIOME_ISLANDS;
    islands.func = GenerateIslandsTerrain;
    islands.skyColor = vec3(0.53f, 0.80f, 0.92f);
    islands.weather.emitter = &state->rain;

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

    Biome& volcanic = world->biomes[BIOME_VOLCANIC];
    volcanic.name = "Volcanic";
    volcanic.type = BIOME_VOLCANIC;
    volcanic.func = GenerateVolcanicTerrain;
    volcanic.skyColor = vec3(0.48f, 0.2f, 0.2f);
    volcanic.weather.emitter = &state->ash;
    volcanic.weather.ambientFade = false;

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

    Biome& dungeon = world->biomes[BIOME_DUNGEON];
    dungeon.name = "Dungeon";
    dungeon.type = BIOME_DUNGEON;
    dungeon.func = GenerateDungeon;
    dungeon.skyColor = vec3(0.0f);
    dungeon.weather.emitter = &state->rain;
}
