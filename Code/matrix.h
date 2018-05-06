// Voxel Engine
// Jason Bricco

#define MatrixToSIMD(matrix) *(float4(*)[4])(&matrix[0][0])

struct Matrix4
{
	Vec4 values[4];

	Matrix4() {}

	Matrix4(float s)
	{
		values[0] = NewV4(s, 0.0f, 0.0f, 0.0f);
		values[1] = NewV4(0.0f, s, 0.0f, 0.0f);
		values[2] = NewV4(0.0f, 0.0f, s, 0.0f);
		values[3] = NewV4(0.0f, 0.0f, 0.0f, s);
	}

	inline Vec4& operator [](int i)
	{
		return values[i];
	}
};

inline float* ValuePtr(Matrix4 m);

static void Mul_MatrixMatrix_SIMD(float4 in1[4], float4 in2[4], float4 out[4]);

inline Matrix4 operator *(Matrix4 a, Matrix4 b)
{
	Matrix4 result;
	Mul_MatrixMatrix_SIMD(MatrixToSIMD(a), MatrixToSIMD(b), MatrixToSIMD(result));
	return result;
}

static Vec4 Mul_MatrixVector(Matrix4 m, Vec4 v);

inline Vec4 operator *(Matrix4 a, Vec4 b)
{
	return Mul_MatrixVector(a, b);
}

static void Inverse_SIMD(float4 in[4], float4 out[4]);
inline Matrix4 Inverse(Matrix4 m);

static Vec3 Unproject(Vec3 winPos, Matrix4 model, Matrix4 proj, Vec4 viewport);
static Matrix4 Perspective(float fovy, float aspect, float zNear, float zFar);
static Matrix4 Ortho(float left, float right, float bottom, float top);

inline Matrix4 Translate(Matrix4 m, Vec3 v);

static Matrix4 LookAt(Vec3 pos, Vec3 target, Vec3 up);
