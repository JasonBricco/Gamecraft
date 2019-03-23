//
// Gamecraft
//

#define FORMAT_CODE(a, b, c, d) (uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t(d) << 24))

#pragma pack(push, 1)

struct AssetFileHeader
{
	uint32_t code;
	uint32_t version;

	uint32_t arrayCount;

	uint32_t imageCount;
	uint32_t images;

	uint32_t soundCount;
	uint32_t sounds;

	uint32_t shaderCount;
	uint32_t shaders;
};

struct ImageData
{
	uint32_t width, height, array;
	uint32_t pixels;
};

struct SoundData
{
	uint32_t channels, sampleCount, sampleRate;
	uint32_t samples;
};

struct ShaderData
{
	uint32_t data;
	uint32_t length;
};

#pragma pack(pop)
