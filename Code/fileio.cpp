// Voxel Engine
// Jason Bricco

static char* PathToAsset(char* fileName)
{
	int size = MAX_PATH * 2;
	char* path = Malloc(char, size, "AssetPath");
	GetModuleFileName(NULL, path, size);

	char* pos = strrchr(path, '\\');
	*(pos + 1) = '\0';

	strcat(path, fileName);
	return path;
}

static char* ReadFileData(char* fileName)
{
	char* path = PathToAsset(fileName);
	char* buffer = NULL;

	ifstream file(path);
	Free(path, "AssetPath");

	if (file)
	{
		file.seekg(0, file.end);
		uint32_t length = (uint32_t)file.tellg() + 1;
		file.seekg(0, file.beg);

		char* inputBuffer = new char[length];
		memset(inputBuffer, 0, length);
		file.read(inputBuffer, length);
		inputBuffer[length - 1] = 0;

		if (inputBuffer) file.close();
		else
		{
			OutputDebugString("Failed to read file!");
			file.close();
			delete[] inputBuffer;
			return NULL;
		}

		buffer = inputBuffer;
		inputBuffer = NULL;
	}
	else
	{
		OutputDebugString("Could not find the file: ");
		OutputDebugString(path);
		return NULL;
	}

	return buffer;
}
