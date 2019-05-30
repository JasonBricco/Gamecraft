//
// Gamecraft
//

enum ImageID
{
	IMAGE_CACTUS_BOTTOM,
	IMAGE_CACTUS_SIDE,
	IMAGE_CACTUS_TOP,
	IMAGE_CLAY,
	IMAGE_COOLED_MAGMA,
	IMAGE_CRATE,
	IMAGE_DIRT,
	IMAGE_GRASS,
	IMAGE_GRASS_SIDE,
	IMAGE_ICE,
	IMAGE_LANTERN_OFF,
	IMAGE_LANTERN_ON,
	IMAGE_LEAVES,
	IMAGE_MAGMA,
	IMAGE_MAGMA2,
	IMAGE_MAGMA3,
	IMAGE_MAGMA4,
	IMAGE_MAGMA5,
	IMAGE_METAL_CRATE,
	IMAGE_OBSIDIAN,
	IMAGE_SAND,
	IMAGE_SNOW,
	IMAGE_SNOW_SIDE,
	IMAGE_STONE,
	IMAGE_STONE_BRICK,
	IMAGE_TRAMPOLINE,
	IMAGE_WATER,
	IMAGE_WATER2,
	IMAGE_WATER3,
	IMAGE_WATER4,
	IMAGE_WATER5,
	IMAGE_WATER6,
	IMAGE_WATER7,
	IMAGE_WATER8,
	IMAGE_WOOD,
	IMAGE_WOOD_TOP,
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
	SHADER_BLOCK_OPAQUE_ANIMATED,
	SHADER_BLOCK_TRANSPARENT,
	SHADER_BLOCK_TRANSPARENT_ANIMATED,
	SHADER_CROSSHAIR,
	SHADER_FADE,
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
