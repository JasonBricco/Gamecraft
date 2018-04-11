// Voxel Engine
// Jason Bricco
// Created March 28, 2018

#define EPSILON 0.0001f

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

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
