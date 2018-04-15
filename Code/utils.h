// Voxel Engine
// Jason Bricco

#define EPSILON 0.001f

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

struct Ray
{
	vec3 origin;
	vec3 dir;
};

static char* PathToAsset(char* fileName);

inline int Clamp(int value, int min, int max);
inline bool Approx(float a, float b);

inline int Square(int value);
inline float Square(float value);

inline int CeilToInt(float value);
inline int RoundToInt(float value);

inline ivec3 BlockPos(vec3 pos);
inline vec3 GetVec3(ivec3 v);
inline vec3 MoveDirXZ(vec3 value);

inline void LogError(char* message);
inline void PrintVec3(vec3 v);
