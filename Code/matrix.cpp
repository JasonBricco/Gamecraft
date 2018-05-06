// Voxel Engine
// Jason Bricco

inline float* ValuePtr(Matrix4 m)
{
	return &(m[0].x);
}

static void Mul_MatrixMatrix_SIMD(float4 in1[4], float4 in2[4], float4 out[4])
{
	{
		float4 e0 = Shuffle_4x(in2[0], in2[0], _MM_SHUFFLE(0, 0, 0, 0));
		float4 e1 = Shuffle_4x(in2[0], in2[0], _MM_SHUFFLE(1, 1, 1, 1));
		float4 e2 = Shuffle_4x(in2[0], in2[0], _MM_SHUFFLE(2, 2, 2, 2));
		float4 e3 = Shuffle_4x(in2[0], in2[0], _MM_SHUFFLE(3, 3, 3, 3));

		float4 m0 = Mul_4x(in1[0], e0);
		float4 m1 = Mul_4x(in1[1], e1);
		float4 m2 = Mul_4x(in1[2], e2);
		float4 m3 = Mul_4x(in1[3], e3);

		float4 a0 = Add_4x(m0, m1);
		float4 a1 = Add_4x(m2, m3);
		float4 a2 = Add_4x(a0, a1);

		out[0] = a2;
	}

	{
		float4 e0 = Shuffle_4x(in2[1], in2[1], _MM_SHUFFLE(0, 0, 0, 0));
		float4 e1 = Shuffle_4x(in2[1], in2[1], _MM_SHUFFLE(1, 1, 1, 1));
		float4 e2 = Shuffle_4x(in2[1], in2[1], _MM_SHUFFLE(2, 2, 2, 2));
		float4 e3 = Shuffle_4x(in2[1], in2[1], _MM_SHUFFLE(3, 3, 3, 3));

		float4 m0 = Mul_4x(in1[0], e0);
		float4 m1 = Mul_4x(in1[1], e1);
		float4 m2 = Mul_4x(in1[2], e2);
		float4 m3 = Mul_4x(in1[3], e3);

		float4 a0 = Add_4x(m0, m1);
		float4 a1 = Add_4x(m2, m3);
		float4 a2 = Add_4x(a0, a1);

		out[1] = a2;
	}

	{
		float4 e0 = Shuffle_4x(in2[2], in2[2], _MM_SHUFFLE(0, 0, 0, 0));
		float4 e1 = Shuffle_4x(in2[2], in2[2], _MM_SHUFFLE(1, 1, 1, 1));
		float4 e2 = Shuffle_4x(in2[2], in2[2], _MM_SHUFFLE(2, 2, 2, 2));
		float4 e3 = Shuffle_4x(in2[2], in2[2], _MM_SHUFFLE(3, 3, 3, 3));

		float4 m0 = Mul_4x(in1[0], e0);
		float4 m1 = Mul_4x(in1[1], e1);
		float4 m2 = Mul_4x(in1[2], e2);
		float4 m3 = Mul_4x(in1[3], e3);

		float4 a0 = Add_4x(m0, m1);
		float4 a1 = Add_4x(m2, m3);
		float4 a2 = Add_4x(a0, a1);

		out[2] = a2;
	}

	{
		float4 e0 = Shuffle_4x(in2[3], in2[3], _MM_SHUFFLE(0, 0, 0, 0));
		float4 e1 = Shuffle_4x(in2[3], in2[3], _MM_SHUFFLE(1, 1, 1, 1));
		float4 e2 = Shuffle_4x(in2[3], in2[3], _MM_SHUFFLE(2, 2, 2, 2));
		float4 e3 = Shuffle_4x(in2[3], in2[3], _MM_SHUFFLE(3, 3, 3, 3));

		float4 m0 = Mul_4x(in1[0], e0);
		float4 m1 = Mul_4x(in1[1], e1);
		float4 m2 = Mul_4x(in1[2], e2);
		float4 m3 = Mul_4x(in1[3], e3);

		float4 a0 = Add_4x(m0, m1);
		float4 a1 = Add_4x(m2, m3);
		float4 a2 = Add_4x(a0, a1);

		out[3] = a2;
	}
}

static Vec4 Mul_MatrixVector(Matrix4 m, Vec4 v)
{
	Vec4 mov0 = NewV4(v[0]);
	Vec4 mov1 = NewV4(v[1]);
	Vec4 mul0 = m[0] * mov0;
	Vec4 mul1 = m[1] * mov1;
	Vec4 add0 = mul0 + mul1;
	Vec4 mov2 = NewV4(v[2]);
	Vec4 mov3 = NewV4(v[3]);
	Vec4 mul2 = m[2] * mov2;
	Vec4 mul3 = m[3] * mov3;
	Vec4 add1 = mul2 + mul3;
	Vec4 add2 = add0 + add1;
	
	return add2;
}

static void Inverse_SIMD(float4 in[4], float4 out[4])
{
	float4 fac0;
	{
		float4 swp0a = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		float4 swp0b = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));

		float4 swp00 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));
		float4 swp01 = Shuffle_4x(swp0a, swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp02 = Shuffle_4x(swp0b, swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp03 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

		float4 mul00 = Mul_4x(swp00, swp01);
		float4 mul01 = Mul_4x(swp02, swp03);
		fac0 = Sub_4x(mul00, mul01);
	}

	float4 fac1;
	{
		float4 swp0a = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		float4 swp0b = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));

		float4 swp00 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
		float4 swp01 = Shuffle_4x(swp0a, swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp02 = Shuffle_4x(swp0b, swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp03 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

		float4 mul00 = Mul_4x(swp00, swp01);
		float4 mul01 = Mul_4x(swp02, swp03);
		fac1 = Sub_4x(mul00, mul01);
	}

	float4 fac2;
	{
		float4 swp0a = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
		float4 swp0b = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));

		float4 swp00 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
		float4 swp01 = Shuffle_4x(swp0a, swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp02 = Shuffle_4x(swp0b, swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp03 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));

		float4 mul00 = Mul_4x(swp00, swp01);
		float4 mul01 = Mul_4x(swp02, swp03);
		fac2 = Sub_4x(mul00, mul01);
	}

	float4 fac3;
	{
		float4 swp0a = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		float4 swp0b = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

		float4 swp00 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		float4 swp01 = Shuffle_4x(swp0a, swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp02 = Shuffle_4x(swp0b, swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp03 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

		float4 mul00 = Mul_4x(swp00, swp01);
		float4 mul01 = Mul_4x(swp02, swp03);
		fac3 = Sub_4x(mul00, mul01);
	}

	float4 fac4;
	{
		float4 swp0a = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
		float4 swp0b = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

		float4 swp00 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		float4 swp01 = Shuffle_4x(swp0a, swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp02 = Shuffle_4x(swp0b, swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp03 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));

		float4 mul00 = Mul_4x(swp00, swp01);
		float4 mul01 = Mul_4x(swp02, swp03);
		fac4 = Sub_4x(mul00, mul01);
	}

	float4 fac5;
	{
		float4 swp0a = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));
		float4 swp0b = Shuffle_4x(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

		float4 swp00 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		float4 swp01 = Shuffle_4x(swp0a, swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp02 = Shuffle_4x(swp0b, swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		float4 swp03 = Shuffle_4x(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));

		float4 mul00 = Mul_4x(swp00, swp01);
		float4 mul01 = Mul_4x(swp02, swp03);
		fac5 = Sub_4x(mul00, mul01);
	}

	float4 signA = Set_4x(1.0f, -1.0f, 1.0f, -1.0f);
	float4 signB = Set_4x(-1.0f, 1.0f, -1.0f, 1.0f);

	float4 temp0 = Shuffle_4x(in[1], in[0], _MM_SHUFFLE(0, 0, 0, 0));
	float4 v0 = Shuffle_4x(temp0, temp0, _MM_SHUFFLE(2, 2, 2, 0));

	float4 temp1 = Shuffle_4x(in[1], in[0], _MM_SHUFFLE(1, 1, 1, 1));
	float4 v1 = Shuffle_4x(temp1, temp1, _MM_SHUFFLE(2, 2, 2, 0));

	float4 temp2 = Shuffle_4x(in[1], in[0], _MM_SHUFFLE(2, 2, 2, 2));
	float4 v2 = Shuffle_4x(temp2, temp2, _MM_SHUFFLE(2, 2, 2, 0));

	float4 temp3 = Shuffle_4x(in[1], in[0], _MM_SHUFFLE(3, 3, 3, 3));
	float4 v3 = Shuffle_4x(temp3, temp3, _MM_SHUFFLE(2, 2, 2, 0));

	float4 mul00 = Mul_4x(v1, fac0);
	float4 mul01 = Mul_4x(v2, fac1);
	float4 Mul02 = Mul_4x(v3, fac2);
	float4 sub00 = Sub_4x(mul00, mul01);
	float4 add00 = Add_4x(sub00, Mul02);
	float4 inv0 = Mul_4x(signB, add00);

	float4 mul03 = Mul_4x(v0, fac0);
	float4 mul04 = Mul_4x(v2, fac3);
	float4 mul05 = Mul_4x(v3, fac4);
	float4 sub01 = Sub_4x(mul03, mul04);
	float4 add01 = Add_4x(sub01, mul05);
	float4 inv1 = Mul_4x(signA, add01);

	float4 mul06 = Mul_4x(v0, fac1);
	float4 mul07 = Mul_4x(v1, fac3);
	float4 mul08 = Mul_4x(v3, fac5);
	float4 sub02 = Sub_4x(mul06, mul07);
	float4 add02 = Add_4x(sub02, mul08);
	float4 inv2 = Mul_4x(signB, add02);

	float4 mul09 = Mul_4x(v0, fac2);
	float4 mul10 = Mul_4x(v1, fac4);
	float4 mul11 = Mul_4x(v2, fac5);
	float4 sub03 = Sub_4x(mul09, mul10);
	float4 add03 = Add_4x(sub03, mul11);
	float4 inv3 = Mul_4x(signA, add03);

	float4 row0 = Shuffle_4x(inv0, inv1, _MM_SHUFFLE(0, 0, 0, 0));
	float4 row1 = Shuffle_4x(inv2, inv3, _MM_SHUFFLE(0, 0, 0, 0));
	float4 row2 = Shuffle_4x(row0, row1, _MM_SHUFFLE(2, 0, 2, 0));

	float4 det0 = Dot_4x(in[0], row2);
	float4 rcp0 = Div_4x(Set1_4x(1.0f), det0);

	out[0] = Mul_4x(inv0, rcp0);
	out[1] = Mul_4x(inv1, rcp0);
	out[2] = Mul_4x(inv2, rcp0);
	out[3] = Mul_4x(inv3, rcp0);
}

inline Matrix4 Inverse(Matrix4 m)
{
	Matrix4 result;
	Inverse_SIMD(MatrixToSIMD(m), MatrixToSIMD(result));
	return result;
}

static Vec3 Unproject(Vec3 winPos, Matrix4 model, Matrix4 proj, Vec4 viewport)
{
	Matrix4 m = proj * model;

	Matrix4 inv = Inverse(m);

	Vec4 temp = NewV4(winPos, 1.0f);
	temp.x = (temp.x - viewport[0]) / viewport[2];
	temp.y = (temp.y - viewport[1]) / viewport[3];
	temp = temp * 2.0f - 1.0f;

	Vec4 obj = inv * temp;
	obj = obj / obj.w;
	
	return NewV3(obj.x, obj.y, obj.z);
}

static Matrix4 Perspective(float fovy, float aspect, float zNear, float zFar)
{
	float tanHalfFovy = tan(fovy / 2.0f);

	Matrix4 result = Matrix4(0.0f);
	result[0][0] = 1.0f / (aspect * tanHalfFovy);
	result[1][1] = 1.0f / tanHalfFovy;
	result[2][3] = -1.0f;
	result[2][2] = -(zFar + zNear) / (zFar - zNear);
	result[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);

	return result;
}

static Matrix4 Ortho(float left, float right, float bottom, float top)
{
	Matrix4 result = Matrix4(1.0f);

	result[0][0] = 2.0f / (right - left);
	result[1][1] = 2.0f / (top - bottom);
	result[2][2] = -1.0f;
	result[3][0] = -(right + left) / (right - left);
	result[3][1] = -(top + bottom) / (top - bottom);

	return result;
}

inline Matrix4 Translate(Matrix4 m, Vec3 v)
{
	m[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
	return m;
}

static Matrix4 LookAt(Vec3 pos, Vec3 target, Vec3 up)
{
	Vec3 f = Normalize(target - pos);
	Vec3 s = Normalize(Cross(f, up));
	Vec3 u = Cross(s, f);

	Matrix4 result = Matrix4(1.0f);

	result[0][0] = s.x;
	result[1][0] = s.y;
	result[2][0] = s.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = -f.x;
	result[1][2] = -f.y;
	result[2][2] = -f.z;
	result[3][0] = -Dot(s, pos);
	result[3][1] = -Dot(u, pos);
	result[3][2] = Dot(f, pos);
	
	return result;
}
