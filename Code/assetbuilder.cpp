//
// Gamecraft
//

#define ASSET_BUILDER

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>

#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#include "stb_image.h"
#include "stb_vorbis.h"

#pragma warning(pop)

using namespace std;

#include "AssetBuilder.h"

#define ArrayLength(array) (sizeof(array) / sizeof((array)[0]))

static void* ReadFileData(const char* path, uint32_t* sizePtr)
{
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER sizeValue;

        if (!GetFileSizeEx(file, &sizeValue))
            return nullptr;

        uint32_t size = (uint32_t)sizeValue.QuadPart;
        void* data = malloc(size);
        
        DWORD bytesRead;

        if (ReadFile(file, data, size, &bytesRead, 0) && (size == bytesRead))
            *sizePtr = size;
        else
        {
        	free(data);
        	data = nullptr;
        }

        CloseHandle(file);
        return data;
    }

    return nullptr;
}

static void PrintData(char* title, char* pre, vector<string>& files)
{
	printf("\n--- %s ---\n", title);

	for (int i = 0; i < files.size(); i++)
	{
		string fStr = string(pre) + files[i];
		fStr = fStr.substr(0, fStr.find_last_of(".")) + ",";

		transform(fStr.begin(), fStr.end(), fStr.begin(), [](char c) { return (char)(::toupper(c)); });
		printf("%s\n", fStr.c_str());
	}
}

static void GetAllFiles(char* path, vector<string>& fileNames)
{
	WIN32_FIND_DATA findData;
	HANDLE handle = FindFirstFile(path, &findData);

	while (handle != INVALID_HANDLE_VALUE)
	{
		fileNames.push_back(string(findData.cFileName));

		if (!FindNextFile(handle, &findData))
			break;
	}

	FindClose(handle);
}

static ImageData WriteImage(FILE* file, char* dir, const char* fileName)
{
	char path[MAX_PATH];
	PathCombine(path, dir, fileName);

	ImageData data = {};
	data.pixels = ftell(file);

	int width, height, components;
	uint8_t* pixels = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
	assert(pixels != nullptr);
	data.width = width;
	data.height = height;

	fwrite(pixels, sizeof(uint8_t), width * height * 4, file);
	
	stbi_image_free(pixels);
	return data;
}

static SoundData WriteSound(FILE* file, char* dir, const char* fileName)
{
	char path[MAX_PATH];
	PathCombine(path, dir, fileName);

	SoundData data = {};
	data.samples = ftell(file);
	int16_t* samples;
	int channels, sampleRate;
	int count = stb_vorbis_decode_filename(path, &channels, &sampleRate, &samples);
	assert(count != -1);
	data.channels = channels;
	data.sampleCount = count;
	data.sampleRate = sampleRate;
	fwrite(samples, sizeof(int16_t), count * 2, file);

	free(samples);
	return data;
}

int main()
{
	AssetFileHeader header = {};
	header.code = FORMAT_CODE('g', 'c', 'a');
	header.version = 1;

	char* blocksPath = "W:\\Assets\\Blocks\\*.png";
	char* blocksDir = "W:\\Assets\\Blocks";

	char* imagePath = "W:\\Assets\\Images\\*.png";
	char* imageDir = "W:\\Assets\\Images";

	char* soundPath = "W:\\Assets\\Sounds\\*.ogg";
	char* soundDir = "W:\\Assets\\Sounds";
	
	char* shaderPaths[2] = 
	{ 
		"W:\\Shaders\\*.vert",
		"W:\\Shaders\\*.frag"
	};

	char* shaderDir = "W:\\Shaders";

	vector<ImageData> blockImages;
	vector<ImageData> images;
	vector<SoundData> sounds;
	vector<ShaderData> shaders;

	FILE* file = fopen("W:/Assets/Assets.gca", "wb");

	if (file != nullptr)
	{
		// Write out block image data.
		fseek(file, sizeof(header), SEEK_SET);
		
		vector<string> blockImageNames;
		GetAllFiles(blocksPath, blockImageNames);

		sort(blockImageNames.begin(), blockImageNames.end());

		for (int i = 0; i < blockImageNames.size(); i++)
			blockImages.push_back(WriteImage(file, blocksDir, blockImageNames[i].c_str()));

		header.blockImages = ftell(file);
		header.blockImageCount = (uint32_t)blockImages.size();
		fwrite(blockImages.data(), sizeof(ImageData), header.blockImageCount, file);

		PrintData("Block Images", "IMAGE_", blockImageNames);

		// Write out image data.
		vector<string> imageNames;
		GetAllFiles(imagePath, imageNames);

		sort(imageNames.begin(), imageNames.end());

		for (int i = 0; i < imageNames.size(); i++)
			images.push_back(WriteImage(file, imageDir, imageNames[i].c_str()));

		header.images = ftell(file);
		header.imageCount = (uint32_t)images.size();
		fwrite(images.data(), sizeof(ImageData), header.imageCount, file);

		PrintData("Images", "IMAGE_", imageNames);

		// Write out sound data.
		vector<string> soundNames;
		GetAllFiles(soundPath, soundNames);

		sort(soundNames.begin(), soundNames.end());

		for (int i = 0; i < soundNames.size(); i++)
			sounds.push_back(WriteSound(file, soundDir, soundNames[i].c_str()));

		header.sounds = ftell(file);
		header.soundCount = (uint32_t)sounds.size();
		fwrite(sounds.data(), sizeof(SoundData), header.soundCount, file);

		PrintData("Sounds", "SOUND_", soundNames);

		// Write out shader data.
		vector<string> vertFiles;
		vector<string> fragFiles;

		GetAllFiles(shaderPaths[0], vertFiles);
		GetAllFiles(shaderPaths[1], fragFiles);

		for (int i = 0; i < vertFiles.size(); i++)
		{
			char vertPath[MAX_PATH];
			PathCombine(vertPath, shaderDir, vertFiles[i].c_str());

			char fragPath[MAX_PATH];
			PathCombine(fragPath, shaderDir, fragFiles[i].c_str());

			char* paths[2] = { vertPath, fragPath };

			for (int p = 0; p < ArrayLength(paths); p++)
			{
				ShaderData shader = {};
				shader.data = ftell(file);
				uint32_t size;
				char* data = (char*)ReadFileData(paths[p], &size);
				shader.length = size;
				fwrite(data, size, 1, file);
				shaders.push_back(shader);
			}
		}

		header.shaders = ftell(file);
		header.shaderCount = (uint32_t)shaders.size();
		fwrite(shaders.data(), sizeof(ShaderData), header.shaderCount, file);

		PrintData("Shaders", "SHADER_", vertFiles);

		fseek(file, 0, SEEK_SET);
		fwrite(&header, sizeof(header), 1, file);

		fclose(file);
	}
	else printf("Failed to open the asset file.\n");
}
