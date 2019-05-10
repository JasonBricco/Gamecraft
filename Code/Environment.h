//
// Gamecraft
//

struct World;
struct ChunkGroup;

struct Weather
{
	ParticleEmitter* emitter;
};

using BiomeFunc = void(*)(World*, ChunkGroup*);

enum BiomeType
{
	BIOME_GRASSY,
	BIOME_SNOW,
	BIOME_DESERT,
	BIOME_FLAT,
	BIOME_GRID,
	BIOME_VOID,
	BIOME_COUNT
};

struct Biome
{
	char* name;
	vec3 skyColor;
	float ambient;
	Weather weather;
	BiomeType type;
	BiomeFunc func;
};

static void CreateBiomes(GameState* state, World* world);
static void ResetEnvironment(GameState* state, World* world);
