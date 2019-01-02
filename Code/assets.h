//
// Jason Bricco
//

enum ImageID
{
	IMAGE_CRATE,
	IMAGE_DIRT,
	IMAGE_GRASS,
	IMAGE_GRASS_SIDE,
	IMAGE_SAND,
	IMAGE_STONE,
	IMAGE_STONE_BRICK,
	IMAGE_WATER,
	IMAGE_CROSSHAIR,
	IMAGE_RAIN,
	IMAGE_COUNT
};

enum ImageArrayID
{
	IMAGE_ARRAY_BLOCKS,
	IMAGE_ARRAY_COUNT
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
	SHADER_FLUID_ARRAY,
	SHADER_FADE,
	SHADER_PARTICLE,
	SHADER_COUNT
};

typedef vector<ImageData> TextureArrayData;

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
	Texture imageArrays[IMAGE_ARRAY_COUNT];
	Sound sounds[SOUND_COUNT];
	Shader shaders[SHADER_COUNT];
};
