//
// Gamecraft
//

enum ImageID
{
	IMAGE_CACTUS_BOTTOM = 0,
	IMAGE_CACTUS_SIDE = 1,
	IMAGE_CACTUS_TOP = 2,
	IMAGE_CLAY = 3,
	IMAGE_COOLED_MAGMA = 4,
	IMAGE_CRATE = 5,
	IMAGE_DIRT = 6,
	IMAGE_GLASS = 7,
	IMAGE_GRASS = 8,
	IMAGE_GRASS_SIDE = 9,
	IMAGE_ICE = 10,
	IMAGE_LANTERN_OFF = 11,
	IMAGE_LANTERN_ON = 12,
	IMAGE_LAVA = 13,
	IMAGE_LEAVES = 21,
	IMAGE_MAGMA = 22,
	IMAGE_METAL_CRATE = 42,
	IMAGE_OBSIDIAN = 43,
	IMAGE_SAND = 44,
	IMAGE_SNOW = 45,
	IMAGE_SNOW_SIDE = 46,
	IMAGE_STONE = 47,
	IMAGE_STONE_BRICK = 48,
	IMAGE_TRAMPOLINE = 49,
	IMAGE_WATER = 50,
	IMAGE_WOOD = 58,
	IMAGE_WOOD_TOP = 59,
	IMAGE_CROSSHAIR,
	IMAGE_GENERIC_PARTICLE,
	IMAGE_RAIN,
	IMAGE_SNOWFLAKE,
	IMAGE_COUNT
};

enum SoundID
{
	SOUND_LEAVES,
	SOUND_STONE,
	SOUND_COUNT
};

enum ShaderID
{
	SHADER_BLOCK_CUTOUT,
	SHADER_BLOCK_OPAQUE,
	SHADER_BLOCK_TRANSPARENT,
	SHADER_CROSSHAIR,
	SHADER_FADE,
	SHADER_OCCLUSION,
	SHADER_PARTICLE,
	SHADER_UI,
	SHADER_COUNT
};

struct Shader
{
    GLuint handle;
    GLint model, view, proj, ambient;
    GLint fadeColor, animIndex;
    GLint fogColor, fogStart, fogEnd;
};

struct Texture
{
	GLuint id;
};

struct Sound
{
	AudioEngine* engine;
	int16_t* samples;
	uint32_t sampleCount;
	uint32_t sampleRate;
};

struct AssetDatabase
{
	Texture images[IMAGE_COUNT];
	Texture blockArray;
	Sound sounds[SOUND_COUNT];
	Shader shaders[SHADER_COUNT];
};
