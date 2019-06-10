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
	IMAGE_GRASS = 7,
	IMAGE_GRASS_SIDE = 8,
	IMAGE_ICE = 9,
	IMAGE_LANTERN_OFF = 10,
	IMAGE_LANTERN_ON = 11,
	IMAGE_LAVA = 12,
	IMAGE_LEAVES = 20,
	IMAGE_MAGMA = 21,
	IMAGE_METAL_CRATE = 41,
	IMAGE_OBSIDIAN = 42,
	IMAGE_SAND = 43,
	IMAGE_SNOW = 44,
	IMAGE_SNOW_SIDE = 45,
	IMAGE_STONE = 46,
	IMAGE_STONE_BRICK = 47,
	IMAGE_TRAMPOLINE = 48,
	IMAGE_WATER = 49,
	IMAGE_WOOD = 57,
	IMAGE_WOOD_TOP = 58,
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
