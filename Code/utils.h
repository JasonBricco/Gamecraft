// Voxel Engine
// Jason Bricco

#define PI 3.141592653f
#define EPSILON 0.001f

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

#define Align16(v) ((v + 15) & ~15)

struct Ray
{
	vec3 origin;
	vec3 dir;
};

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

inline bool InRange(float val, float min, float max) 
{
	return val >= min && val <= max;
}

inline int InRange(int val, int min, int max)
{
	return val >= min && val <= max;
}

inline vec3 MoveDirXZ(vec3 value)
{
	vec3 zeroY = vec3(value.x, 0.0f, value.z);
	return normalize(zeroY);
}

inline ivec3 BlockPos(vec3 pos)
{
	return { RoundToInt(pos.x), RoundToInt(pos.y), RoundToInt(pos.z) };
}

inline vec3 GetV3(ivec3 v)
{
	return { v.x, v.y, v.z };
}

inline float Lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

inline float SCurve3(float a)
{
	return (a * a * (3.0f - 2.0f * a));
}

struct Rectf
{
    vec3 min;
    vec3 max;
};

inline Rectf NewRect(vec3 min, vec3 max)
{
    return { min, max };
}
