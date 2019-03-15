//
// Jason Bricco
//

static void TestDiagonalCollision()
{
	AABB a = AABBFromCorner(vec3(5.0f), vec3(2.0f));
	AABB b = AABBFromCorner(vec3(10.0f), vec3(2.0f));

	vec3 vel = vec3(2.0f);
	vec3 normal;
	float result = SweptAABB(a, b, vel, normal);
	AssertEquals(result, 1.0f);

	vel = vec3(4.0f);
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);

	vel = vec3(10.0f);
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);
}

static void TestNegativeDiagonalCollision()
{
	AABB a = AABBFromCorner(vec3(-5.0f), vec3(2.0f));
	AABB b = AABBFromCorner(vec3(-10.0f), vec3(2.0f));

	vec3 vel = vec3(-2.0f);
	vec3 normal;
	float result = SweptAABB(a, b, vel, normal);
	AssertEquals(result, 1.0f);

	vel = vec3(-4.0f);
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);

	vel = vec3(-10.0f);
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);
}

static void TestPositiveCollision(int axis)
{
	AABB a = AABBFromCorner(vec3(0.0f), vec3(1.0f));

	vec3 cornerB = vec3(0.0f);
	cornerB[axis] = 5.0f;
	AABB b = AABBFromCorner(cornerB, vec3(1.0f));

	vec3 vel = vec3(0.0f);
	vel[axis] = 3.0f;
	vec3 normal;
	float result = SweptAABB(a, b, vel, normal);
	AssertEquals(result, 1.0f);

	vel[axis] = 4.05f;
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);
	AssertEquals(normal[axis], -1.0f);

	vel[axis] = 20.0f;
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);
	AssertEquals(normal[axis], -1.0f);
}

static void TestNegativeCollision(int axis)
{
	AABB a = AABBFromCorner(vec3(0.0f), vec3(1.0f));

	vec3 cornerB = vec3(0.0f);
	cornerB[axis] = -5.0f;
	AABB b = AABBFromCorner(cornerB, vec3(1.0f));

	vec3 vel = vec3(0.0f);
	vel[axis] = -3.0f;
	vec3 normal;
	float result = SweptAABB(a, b, vel, normal);
	AssertEquals(result, 1.0f);

	vel[axis] = -4.05f;
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);
	AssertEquals(normal[axis], 1.0f);

	vel[axis] = -20.0f;
	result = SweptAABB(a, b, vel, normal);
	AssertNotEquals(result, 1.0f);
	AssertEquals(normal[axis], 1.0f);
}

static void TestCollision()
{
	TestDiagonalCollision();
	TestNegativeDiagonalCollision();

	TestPositiveCollision(0);
	TestPositiveCollision(1);
	TestPositiveCollision(2);

	TestNegativeCollision(0);
	TestNegativeCollision(1);
	TestNegativeCollision(2);

	AABB a = AABBFromCorner(vec3(5.0f), vec3(1.0f));
	AABB ground = AABBFromCorner(vec3(0.0f), vec3(10.0f, 1.0f, 10.0f));

	vector<AABB> collides;
	collides.push_back(ground);

	vec3 delta = vec3(10.0f, 20.0f, 0.0f);
	vec3 vel = delta;
	vec3 pos = a.pos;

	ProcessCollisions(a, collides, pos, vel, delta);
}
