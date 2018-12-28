//
// Jason Bricco
//

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdint.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#include "stb_image.h"
#include "stb_vorbis.h"

#pragma warning(pop)

using namespace std;

#include "assetbuilder.h"
#include "memory.h"
#include "filehelper.h"

#define ArrayLength(array) (sizeof(array) / sizeof((array)[0]))

int main()
{
	AssetFileHeader header = {};
	header.code = FORMAT_CODE('g', 'c', 'a', 'f');
	header.version = 1;
	header.arrayCount = 1;

	char* imagePaths[] = 
	{
		"W:/Assets/Crate.png",
		"W:/Assets/Dirt.png",
		"W:/Assets/Grass.png",
		"W:/Assets/GrassSide.png",
		"W:/Assets/Sand.png",
		"W:/Assets/Stone.png",
		"W:/Assets/StoneBrick.png",
		"W:/Assets/Water.png",
		"W:/Assets/Crosshair.png"
	};

	// Specifies which array each of the above images should be within. 
	// INT_MAX means the image doesn't belong in an array.
	int arrayIndices[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, INT_MAX
	};

	char* soundPaths[] = 
	{
		"W:/Assets/Leaves.ogg",
		"W:/Assets/Stone.ogg"
	};

	char* shaderPaths[] = 
	{
		"W:/Shaders/crosshair.vert",
		"W:/Shaders/crosshair.frag",
		"W:/Shaders/diffuse_array.vert",
		"W:/Shaders/diffuse_array.frag",
		"W:/Shaders/fluid_array.vert",
		"W:/Shaders/fluid_array.frag",
		"W:/Shaders/fade.vert",
		"W:/Shaders/fade.frag"
	};

	const int imageCount = ArrayLength(imagePaths);
	const int soundCount = ArrayLength(soundPaths);
	const int shaderCount = ArrayLength(shaderPaths);

	header.imageCount = imageCount;
	header.soundCount = soundCount;
	header.shaderCount = shaderCount;

	header.images = sizeof(header);
	header.sounds = header.images + header.imageCount * sizeof(ImageData);
	header.shaders = header.sounds + header.soundCount * sizeof(SoundData);

	ImageData images[imageCount];
	SoundData sounds[soundCount];
	ShaderData shaders[shaderCount];

	FILE* file = fopen("W:/Assets/Assets.gca", "wb");

	if (file != nullptr)
	{
		fwrite(&header, sizeof(header), 1, file);
		fseek(file, header.shaders + header.shaderCount * sizeof(ShaderData), SEEK_SET);

		for (int i = 0; i < imageCount; i++)
		{
			images[i].pixels = ftell(file);
			int width, height, components;
			uint8_t* pixels = stbi_load(imagePaths[i], &width, &height, &components, STBI_rgb_alpha);
			assert(pixels != nullptr);
			images[i].width = width;
			images[i].height = height;
			images[i].array = arrayIndices[i];
			fwrite(pixels, sizeof(uint8_t), width * height * 4, file);
			stbi_image_free(pixels);
		}

		for (int i = 0; i < soundCount; i++)
		{
			sounds[i].samples = ftell(file);
			int16_t* samples;
			int channels, sampleRate;
			int count = stb_vorbis_decode_filename(soundPaths[i], &channels, &sampleRate, &samples);
			assert(count != -1);
			sounds[i].channels = channels;
			sounds[i].sampleCount = count;
			sounds[i].sampleRate = sampleRate;
			fwrite(samples, sizeof(int16_t), count * 2, file);
			free(samples);
		}

		for (int i = 0; i < shaderCount; i++)
		{
			shaders[i].data = ftell(file);
			int size;
			char* data = (char*)ReadFileData(shaderPaths[i], &size);
			shaders[i].length = size;
			fwrite(data, size, 1, file);
		}

		fseek(file, header.images, SEEK_SET);
		fwrite(&images, sizeof(ImageData), imageCount, file);

		fseek(file, header.sounds, SEEK_SET);
		fwrite(&sounds, sizeof(SoundData), soundCount, file);

		fseek(file, header.shaders, SEEK_SET);
		fwrite(&shaders, sizeof(ShaderData), shaderCount, file);
		
		fclose(file);
	}
	else printf("Failed to open the asset file.\n");
}
