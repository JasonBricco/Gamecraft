//
// Jason Bricco
//

static void TestDiagonalCollision()
{
	AABB a = AABBFromCorner(vec3(5.0f), vec3(2.0f));
	AABB b = AABBFromCorner(vec3(10.0f), vec3(2.0f));

	float tMin = 1.0f;
	vec3 delta = vec3(2.0f);
	TestCollision(a, b, delta, tMin);
	AssertEquals(tMin, 1.0f);

	tMin = 1.0f;
	delta = vec3(5.5f);
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);

	tMin = 1.0f;
	delta = vec3(10.0f);
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);
}

static void TestNegativeDiagonalCollision()
{
	AABB a = AABBFromCorner(vec3(-5.0f), vec3(2.0f));
	AABB b = AABBFromCorner(vec3(-10.0f), vec3(2.0f));

	float tMin = 1.0f;
	vec3 delta = vec3(-2.0f);
	TestCollision(a, b, delta, tMin);
	AssertEquals(tMin, 1.0f);

	tMin = 1.0f;
	delta = vec3(-5.5f);
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);

	tMin = 1.0f;
	delta = vec3(-10.0f);
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);
}

static void TestPositiveCollision(int axis)
{
	AABB a = AABBFromCorner(vec3(0.0f), vec3(1.0f));

	vec3 cornerB = vec3(0.0f);
	cornerB[axis] = 5.0f;
	AABB b = AABBFromCorner(cornerB, vec3(1.0f));

	vec3 delta = vec3(0.0f);
	float tMin = 1.0f;
	delta[axis] = 3.0f;
	TestCollision(a, b, delta, tMin);
	AssertEquals(tMin, 1.0f);

	delta[axis] = 5.05f;
	tMin = 1.0f;
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);

	delta[axis] = 20.0f;
	tMin = 1.0f;
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);
}

static void TestNegativeCollision(int axis)
{
	AABB a = AABBFromCorner(vec3(0.0f), vec3(1.0f));

	vec3 cornerB = vec3(0.0f);
	cornerB[axis] = -5.0f;
	AABB b = AABBFromCorner(cornerB, vec3(1.0f));

	vec3 delta = vec3(0.0f);
	float tMin = 1.0f;
	delta[axis] = -3.0f;
	TestCollision(a, b, delta, tMin);
	AssertEquals(tMin, 1.0f);

	delta[axis] = -5.05f;
	tMin = 1.0f;
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);

	delta[axis] = -20.0f;
	tMin = 1.0f;
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);
}

static void TestSmallCollision()
{
	AABB a = AABBFromCorner(vec3(0.0f), vec3(0.6f, 1.8f, 0.6f));
	AABB b = AABBFromCorner(vec3(0.61f, 0.0f, 0.0f), vec3(1.0f));

	vec3 delta = vec3(0.02f, 0.0f, 0.0f);
	float tMin = 1.0f;
	TestCollision(a, b, delta, tMin);
	AssertNotEquals(tMin, 1.0f);
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

	TestSmallCollision();
}
