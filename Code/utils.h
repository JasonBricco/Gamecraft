// Voxel Engine
// Jason Bricco

#define PI 3.141592653f
#define EPSILON 0.001f

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

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

inline float Clamp(float value, float min, float max)
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

inline float InverseSqrt(float v)
{
	return 1.0f / sqrt(v);
}

inline int CeilToInt(float value)
{
	return (int)ceilf(value);
}

inline int RoundToInt(float value)
{
	return (int)roundf(value);
}

inline int32_t FloorToInt(float value)
{
    return (int32_t)floorf(value);
}

inline bool Approx(float a, float b)
{
    return abs(a - b) < EPSILON;
}

inline float Radians(float degrees)
{
	return degrees * 0.01745329f;
}
