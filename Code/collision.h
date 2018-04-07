// Voxel Engine
// Jason Bricco

struct AABB
{
	vec3 center;
	float rx, ry, rz;
};

// Test to see if two AABBs overlap.
static bool OverlapAABB(AABB a, AABB b);

inline AABB NewAABB(vec3 center, float rx, float ry, float rz);
inline AABB MoveAABB(AABB a, vec3 amount);
inline bool OverlapBlock(AABB a, int bX, int bY, int bZ);

// Sphere with center c and radius r.
struct Sphere
{
	vec3 c;
	float r;
};

inline Sphere NewSphere(vec3 center, float r);

// Test to see if two spheres overlap.
static bool OverlapSphere(Sphere a, Sphere b);

// Capsule from points a and b and radius r. 
struct Capsule
{
	vec3 a;
	vec3 b;
	float r;
};

// Test to see if two capsules overlap.
static bool OverlapCapsule(Capsule a, Capsule b);

// Test to see if a sphere overlaps with a capsule.
static bool OverlapSphereCapsule(Sphere s, Capsule capsule);

struct Slab
{
	vec3 normal;

	// Signed distance from origin for the near plane.
	float dNear;

	// Signed distance from origin for the far plane.
	float dFar;
};

struct Plane
{
	// Plane's normal. Points on the plane satisfy dot(n, x) = d;
	vec3 n; 

	// d = dot(n, p) for a given point, p, on the plane.
	float d;
};

// Compute the plane given three noncollinear points (ordered counterclockwise).
static Plane ComputePlane(vec3 a, vec3 b, vec3 c);

// Returns the signed distance of point a to the plane.
inline float DistPointPlane(vec3 a, Plane p)

inline vec3 ClosestPointOnPlane(vec3 a, Plane p);

// Computes the signed area of a triangle;
inline float TriArea(float x1, float y1, float x2, float y2, float x3, float y3);

// Compute barycentric coordinates (u, v, w) for point p with respect to triangle (a, b, c).
static void Barycentric(vec3 a, vec3 b, vec3 c, vec3 p, float* u, float* v, float* w);

// Determines if a point, p, is contained within the triangle (a, b, c).
static int PointInTriangle(vec3 p, vec3 a, vec3 b, vec3 c);
