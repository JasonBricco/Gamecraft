// Voxel Engine
// Jason Bricco

static char* PathToAsset(char* fileName)
{
	int size = MAX_PATH * 2;
	char* path = (char*)malloc(size);
	GetModuleFileName(NULL, path, size);

	char* pos = strrchr(path, '\\');
	*(pos + 1) = '\0';

	strcat(path, fileName);
	return path;
}

inline int Clamp(int value, int min, int max)
{
	return value <= min ? min : value >= max ? max : value;
}

inline int Square(int value)
{
	return value * value;
}

inline float Square(float value)
{
	return value * value;
}

inline int CeilToInt(float value)
{
	return (int)ceilf(value);
}

inline int RoundToInt(float value)
{
	return (int)roundf(value);
}

inline ivec3 BlockPos(vec3 pos)
{
	return ivec3(RoundToInt(pos.x), RoundToInt(pos.y), RoundToInt(pos.z));
}

inline bool Approx(float a, float b)
{
    return abs(a - b) < EPSILON;
}

inline vec3 GetVec3(ivec3 v)
{
	return vec3(v.x, v.y, v.z);
}

inline vec3 MoveDirXZ(vec3 value)
{
	vec3 zeroY = vec3(value.x, 0.0f, value.z);
	return normalize(zeroY);
}

inline void LogError(char* message)
{
	fprintf(stderr, "%s\n", message);
}

inline void PrintVec3(vec3 v)
{
	printf("(%.02f, %.02f, %.02f)\n", v.x, v.y, v.z);
}
