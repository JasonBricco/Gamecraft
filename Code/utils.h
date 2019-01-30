//
// Jason Bricco
//

#define PI 3.141592653f
#define EPSILON 0.001f

#define ArrayLength(array) (sizeof(array) / sizeof((array)[0]))

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

#define Align4(v) ((v + 3) & ~3)
#define Align8(v) ((v + 7) & ~7)
#define Align16(v) ((v + 15) & ~15)

typedef vec4 Color;

#define RED_COLOR vec4(1.0f, 0.0f, 0.0f, 1.0f)
#define GREEN_COLOR vec4(0.0f, 1.0f, 0.0f, 1.0f)
#define BLUE_COLOR vec4(0.0f, 0.0f, 1.0f, 1.0f)
#define CLEAR_COLOR vec4(0.0f, 0.0f, 0.0f, 0.0f)
#define BLACK_COLOR vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define WHITE_COLOR vec4(1.0f, 1.0f, 1.0f, 1.0f)

struct Colori
{
    uint8_t r, g, b, a;
};

static inline Colori NewColori(uint8_t value)
{
    return { value, value, value, value };
}

static inline Colori NewColori(int r, int g, int b, int a)
{
    return { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
}

enum Axis { AXIS_X, AXIS_Y, AXIS_Z };

struct Ray
{
    vec3 origin;
    vec3 dir;
};

#define DIRECTION_LEFT 0
#define DIRECTION_RIGHT 1
#define DIRECTION_BACK 2
#define DIRECTION_FRONT 3
#define DIRECTION_UP 4
#define DIRECTION_DOWN 5

static const ivec3 DIRECTIONS_2D[8] = 
{ 
    ivec3(-1, 0, 0), ivec3(1, 0, 0), ivec3(0, 0, -1), ivec3(0, 0, 1),
    ivec3(-1, 0, -1), ivec3(-1, 0, 1), ivec3(1, 0, -1), ivec3(1, 0, 1)
};

static const ivec3 DIRECTIONS_3D[6] =
{
    ivec3(-1, 0, 0), ivec3(1, 0, 0), ivec3(0, 0, -1), ivec3(0, 0, 1), ivec3(0, 1, 0), ivec3(0, -1, 0)
};

static inline int Clamp(int value, int min, int max)
{
    return value <= min ? min : value >= max ? max : value;
}

static inline float Clamp(float value, float min, float max)
{
    return value <= min ? min : value >= max ? max : value;
}

static inline int Square(int value)
{
    return value * value;
}

static inline float Square(float value)
{
    return value * value;
}

static inline float InverseSqrt(float v)
{
    return 1.0f / sqrt(v);
}

static inline int CeilToInt(float value)
{
    return (int)ceilf(value);
}

static inline int RoundToInt(float value)
{
    return (int)roundf(value);
}

static inline int FloorToInt(float value)
{
    return (int)floorf(value);
}

static inline bool Approx(float a, float b)
{
    return abs(a - b) < EPSILON;
}

static inline bool InRange(float val, float min, float max) 
{
    return val >= min && val <= max;
}

static inline int InRange(int val, int min, int max)
{
    return val >= min && val <= max;
}

static inline vec3 GetXZ(vec3 value)
{
    return vec3(value.x, 0.0f, value.z);
}

static inline ivec3 BlockPos(vec3 pos)
{
    return { RoundToInt(pos.x), RoundToInt(pos.y), RoundToInt(pos.z) };
}

static inline vec3 GetV3(ivec3 v)
{
    return { v.x, v.y, v.z };
}

struct LerpData
{
    float start, end;
    float t, time;
};

// Linear interpolation.
static inline float Lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static inline float SCurve3(float a)
{
    return (a * a * (3.0f - 2.0f * a));
}

static inline Color AverageColor(Color first, Color second, Color third, Color fourth)
{
    float r = (first.r + second.r + third.r + fourth.r) / 4.0f;
    float b = (first.b + second.b + third.b + fourth.b) / 4.0f;
    float g = (first.g + second.g + third.g + fourth.g) / 4.0f;
    float a = (first.a + second.a + third.a + fourth.a) / 4.0f;
    
    return vec4(r, b, g, a);
}

static inline Color AverageColor(Color first, Color second, Color third)
{
    float r = (first.r + second.r + third.r) / 3.0f;
    float b = (first.b + second.b + third.b) / 3.0f;
    float g = (first.g + second.g + third.g) / 3.0f;
    float a = (first.a + second.a + third.a) / 3.0f;
    
    return vec4(r, b, g, a);
}

static inline Colori AverageColor(Colori first, Colori second, Colori third, Colori fourth)
{
    int r = (first.r + second.r + third.r + fourth.r) >> 2;
    int b = (first.b + second.b + third.b + fourth.b) >> 2;
    int g = (first.g + second.g + third.g + fourth.g) >> 2;
    int a = (first.a + second.a + third.a + fourth.a) >> 2;
    
    return NewColori(r, g, b, a);
}

static inline Colori AverageColor(Colori first, Colori second, Colori third)
{
    int r = (first.r + second.r + third.r) / 3;
    int b = (first.b + second.b + third.b) / 3;
    int g = (first.g + second.g + third.g) / 3;
    int a = (first.a + second.a + third.a) / 3;
    
    return NewColori(r, b, g, a);
}

struct Rectf
{
    vec3 min;
    vec3 max;
};

static inline Rectf NewRect(vec3 min, vec3 max)
{
    return { min, max };
}

struct Recti
{
    ivec3 min;
    ivec3 max;
};

static inline Recti NewRect(ivec3 min, ivec3 max)
{
    return { min, max };
}

static inline bool Intersects(ivec3 pos, Recti rect)
{
    return pos.x >= rect.min.x && pos.z >= rect.min.z && pos.x < rect.max.x && pos.z < rect.max.z;
}

// Implements hashing and equality checking operations for the ivec3 type.
// This allows it to be a key in a map.
struct ivec3Key
{
    size_t operator()(const ivec3& k) const
    {
        return 31 * hash<int>()(k.x) + 17 * hash<int>()(k.y) + 13 * hash<int>()(k.z);
    }

    bool operator()(const ivec3& a, const ivec3& b) const
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

