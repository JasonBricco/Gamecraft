//
// Gamecraft
//

enum ImageID
{
	IMAGE_CLAY,
	IMAGE_CRATE,
	IMAGE_DIRT,
	IMAGE_GRASS,
	IMAGE_GRASS_SIDE,
	IMAGE_ICE,
	IMAGE_LEAVES,
	IMAGE_METAL_CRATE,
	IMAGE_SAND,
	IMAGE_SNOW,
	IMAGE_SNOW_SIDE,
	IMAGE_STONE,
	IMAGE_STONE_BRICK,
	IMAGE_WATER,
	IMAGE_WATER2,
	IMAGE_WATER3,
	IMAGE_WATER4,
	IMAGE_WOOD_SIDE,
	IMAGE_WOOD_TOP,
	IMAGE_CROSSHAIR,
	IMAGE_RAIN,
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
	SHADER_CROSSHAIR,
	SHADER_DIFFUSE_ARRAY,
	SHADER_FADE,
	SHADER_FLUID_ARRAY,
	SHADER_PARTICLE,
	SHADER_UI,
	SHADER_COUNT
};

struct Shader
{
    GLuint handle;
    GLint model, view, proj, ambient;
    GLint time, fadeColor;
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
