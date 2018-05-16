// Voxel Engine
// Jason Bricco

#define PI 3.141592653f
#define EPSILON 0.001f

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

#define Align16(v) ((v + 15) & ~15)

typedef vec4 Color;

enum Axis { AXIS_X, AXIS_Y, AXIS_Z };

struct Ray
{
	vec3 origin;
	vec3 dir;
};

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

inline Color Average(Color first, Color second, Color third, Color fourth)
{
    float r = (first.r + second.r + third.r + fourth.r) / 4.0f;
    float b = (first.b + second.b + third.b + fourth.b) / 4.0f;
    float g = (first.g + second.g + third.g + fourth.g) / 4.0f;
    float a = (first.a + second.a + third.a + fourth.a) / 4.0f;
    
    return vec4(r, b, g, a);
}

inline Color Average(Color first, Color second, Color third)
{
    float r = (first.r + second.r + third.r) / 3.0f;
    float b = (first.b + second.b + third.b) / 3.0f;
    float g = (first.g + second.g + third.g) / 3.0f;
    float a = (first.a + second.a + third.a) / 3.0f;
    
    return vec4(r, b, g, a);
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
