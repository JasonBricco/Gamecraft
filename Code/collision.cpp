// Voxel Engine
// Jason Bricco

inline AABB NewAABB(vec3 center, float rx, float ry, float rz)
{
	return { center, rx, ry, rz };
}

static bool OverlapAABB(AABB a, AABB b)
{
	if (abs(a.rx - b.rx) > (a.rx + b.rx)) return false;
	if (abs(a.ry - b.ry) > (a.ry + b.ry)) return false;
	if (abs(a.rz - b.rz) > (a.rz + b.rz)) return false;

	return true;
}

inline AABB MoveAABB(AABB a, vec3 amount)
{
	return NewAABB(a.center + amount, a.rx, a.ry, a.rz);
}

inline bool OverlapBlock(AABB a, int bX, int bY, int bZ)
{
	return OverlapAABB(a, NewAABB(vec3(bX, bY, bZ), 1.0f, 1.0f, 1.0f));
}

inline Sphere NewSphere(vec3 center, float r)
{
	return { center, r };
}

static bool OverlapSphere(Sphere a, Sphere b)
{
	// Squared distance between sphere centers.
	vec3 d = a.c - b.c;
	float dist2 = dot(d, d);

	// If squared distance is less than the sum of the radii, an intersection occurred.
	float sum = a.r + b.r;
	return dist2 <= sum * sum;
}

#if 0
static bool OverlapSphereCapsule(Sphere s, Capsule capsule)
{
	// Compute squared distance between the sphere center and capsule line segment.
	float dist2 = SqDist(capsule.a, capsule.b, s.c);

	// Collision if the squared distance is smalle than the squared sum of the radii.
	float radius = s.r + capsule.r;
	return dist2 <= radius * radius;
}

static bool OverlapCapsule(Capsule a, Capsule b)
{
	// Compute the squared distance between the inner capsule structures.
	float s, t;
	vec3 c1, c2;
	float dist2 = ClosestPoint(a.a, a.b, b.a, b.b, s, t, c1, c2);

	// If squared distance is smaller than the squared sum of the radii, they collide.
	float radius = a.r + b.r;
	return dist2 <= radius * radius;
}
#endif

static Plane ComputePlane(vec3 a, vec3 b, vec3 c)
{
	Plane p;
	p.n = normalize(cross(b - a, c - a));
	p.d = dot(p.n, a);
	return p;
}

inline vec3 ClosestPointOnPlane(vec3 a, Plane p)
{
	return a - DistPointPlane(a, p) * p.n;
}

inline float DistPointPlane(vec3 a, Plane p)
{
	return (dot(p.n, q) - p.d);
}

inline float TriArea(float x1, float y1, float x2, float y2, float x3, float y3)
{
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
}

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

static int PointInTriangle(vec3 p, vec3 a, vec3 b, vec3 c)
{
	float u, v, w;
	Barycentric(a, b, c, p, &u, &v, &w);

	return v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f;
}
