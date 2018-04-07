// Voxel Engine
// Jason Bricco

struct AABB
{
	vec3 center;
	float rx, ry, rz;
};

static bool TestAABB(AABB a, AABB b);
inline AABB NewAABB(vec3 center, float rx, float ry, float rz);
inline AABB MoveAABB(AABB a, vec3 amount);
inline bool TestBlock(AABB a, int bX, int bY, int bZ);

struct Plane
{
	// Plane's normal. Points on the plane satisfy dot(n, x) = d;
	vec3 n; 

	// d = dot(n, p) for a given point, p, on the plane.
	float d;
};

// Compute the plane given three noncollinear points (ordered counterclockwise).
static Plane ComputePlane(vec3 a, vec3 b, vec3 c);

// Computes the signed area of a triangle;
inline float TriArea(float x1, float y1, float x2, float y2, float x3, float y3);

// Compute barycentric coordinates (u, v, w) for point p with respect to triangle (a, b, c).
static void Barycentric(vec3 a, vec3 b, vec3 c, vec3 p, float* u, float* v, float* w);

// Determines if a point, p, is contained within the triangle (a, b, c).
static int PointInTriangle(vec3 p, vec3 a, vec3 b, vec3 c);
