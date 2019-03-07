//
// Jason Bricco
//

struct World;
struct ChunkGroup;

using BiomeFunc = void(*)(World*, ChunkGroup*);

enum BiomeType
{
	BIOME_GRASSY,
	BIOME_SNOW,
	BIOME_FLAT,
	BIOME_GRID,
	BIOME_COUNT
};

struct Biome
{
	char* name;
	BiomeType type;
	BiomeFunc func;
};

static void CreateBiomes(World* world);
