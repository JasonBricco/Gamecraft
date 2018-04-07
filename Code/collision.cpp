// Voxel Engine
// Jason Bricco
// Created April 3, 2018

struct AABB
{
	vec3 center;
	float rx, ry, rz;
};

static bool TestAABB(AABB a, AABB b)
{
	if (abs(a.rx - b.rx) > (a.rx + b.rx)) return false;
	if (abs(a.ry - b.ry) > (a.ry + b.ry)) return false;
	if (abs(a.rz - b.rz) > (a.rz + b.rz)) return false;

	return true;
}

inline AABB NewAABB(vec3 center, float rx, float ry, float rz)
{
	return { center, rx, ry, rz };
}

inline AABB MoveAABB(AABB a, vec3 amount)
{
	return NewAABB(a.center + amount, a.rx, a.ry, a.rz);
}

inline bool TestBlock(AABB a, int bX, int bY, int bZ)
{
	return TestAABB(a, NewAABB(vec3(bX, bY, bZ), 1.0f, 1.0f, 1.0f));
}

struct Plane
{
	// Plane's normal. Points on the plane satisfy dot(n, x) = d;
	vec3 n; 

	// d = dot(n, p) for a given point, p, on the plane.
	float d;
};

// Compute the plane given three noncollinear points (ordered counterclockwise).
static Plane ComputePlane(vec3 a, vec3 b, vec3 c)
{
	Plane p;
	p.n = normalize(cross(b - a, c - a));
	p.d = dot(p.n, a);
	return p;
}

// Computes the signed area of a triangle (page 52).
inline float TriArea(float x1, float y1, float x2, float y2, float x3, float y3)
{
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
}

// Compute barycentric coordinates (u, v, w) for point p with respect to triangle (a, b, c).
static void Barycentric(vec3 a, vec3 b, vec3 c, vec3 p, float* u, float* v, float* w)
{
	// Unnormalized triangle normal.
	vec3 m = cross(b - a, c - a);

	// Nominators and one-over-denominator for u and v ratios.
	float nu, nv, ood;

	// Absolute components for determining projection plane.
	float x = abs(m.x), y = abs(m.y), z = abs(m.z);

	// Compute areas in the plane of largest projection. 
	if (x >= y && x >= z)
	{
		// X is largest, project to the yz plane.
		// Area of PBC in yz plane.
		nu = TriArea(p.y, p.z, b.y, b.z, c.y, c.z); 

		// Area of PCA in yz plane.
		nv = TriArea(p.y, p.z, c.y, c.z, a.y, a.z);

		// 1 / (2 * area of ABC in yz plane).
		ood = 1.0f / m.x;
	}
	else if (y >= x && y >= z)
	{
		// Y is largest, project to the xz plane.
		nu = TriArea(p.x, p.z, b.x, b.z, c.x, c.z);
		nv = TriArea(p.x, p.z, c.x, c.z, a.x, a.z);
		ood = 1.0f / -m.y;
	}
	else
	{
		// Z is largest, project to the xy plane.
		nu = TriArea(p.x, p.y, b.x, b.y, c.x, c.y);
		nv = TriArea(p.x, p.y, c.x, c.y, a.x, a.y);
		ood = 1.0f / m.z;
	}

	*u = nu * ood;
	*v = nv * ood;
	*w = 1.0f - *u - *v;
}

// Determines if a point, p, is contained within the triangle (a, b, c).
static int PointInTriangle(vec3 p, vec3 a, vec3 b, vec3 c)
{
	float u, v, w;
	Barycentric(a, b, c, p, &u, &v, &w);

	return v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f;
}
