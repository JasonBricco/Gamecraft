//
// Gamecraft
//

struct World;
struct ChunkGroup;

struct Weather
{
	ParticleEmitter* emitter;
	bool ambientFade = true;
};

using BiomeFunc = void(*)(World*, ChunkGroup*);

enum BiomeType
{
	BIOME_FOREST,
	BIOME_ISLANDS,
	BIOME_SNOW,
	BIOME_DESERT,
	BIOME_VOLCANIC,
	BIOME_FLAT,
	BIOME_GRID,
	BIOME_VOID,
	BIOME_DUNGEON,
	BIOME_COUNT
};

struct Biome
{
	char* name;
	vec3 skyColor;
	Weather weather;
	BiomeType type;
	BiomeFunc func;
};

static void CreateBiomes(GameState* state, World* world);
static void ResetEnvironment(GameState* state, World* world);
