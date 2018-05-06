// Voxel Engine
// Jason Bricco

#define VecToFloat4(vec) *(float4)(&vec[0])

struct Vec2
{
	float x, y;
};

inline Vec2 NewV2(float x, float y)
{
	return { x, y };
}

struct Vec2i
{
	int x, y;
};

inline Vec2i NewV2i(int x, int y)
{
	return { x, y };
}

struct Vec3
{
	float x, y, z;

	inline float& operator [](int i)
	{
		return (&x)[i];
	}
};

inline Vec3 NewV3(float v)
{
	return { v, v, v };
}

inline Vec3 NewV3(float x, float y, float z)
{
	return { x, y, z };
}

inline Vec3 NewV3(Vec2i v, float z)
{
	return { (float)v.x, (float)v.y, z };
}

inline Vec3 NewV3(Vec2 v, float z)
{
	return { v.x, v.y, z };
}

inline Vec3 operator *(Vec3 a, Vec3 b)
{
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

inline Vec3 operator *(Vec3 v, float s)
{
	return { v.x * s, v.y * s, v.z * s };
}

inline Vec3 operator +(Vec3 a, Vec3 b)
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vec3 operator +(Vec3 v, float s)
{
	return { v.x + s, v.y + s, v.z + s };
}

inline Vec3 operator -(Vec3 a, Vec3 b)
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vec3 operator -(Vec3 v)
{
	return { -v.x, -v.y, -v.z };
}

inline bool operator ==(Vec3 a, Vec3 b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline float Dot(Vec3 a, Vec3 b)
{
	Vec3 temp = a * b;
	return temp.x + temp.y + temp.z;
}

inline Vec3 Cross(Vec3 a, Vec3 b)
{
	return NewV3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y);
}

inline Vec3 Normalize(Vec3 v)
{
	return v * InverseSqrt(Dot(v, v));
}

inline float Length(Vec3 v)
{
	return sqrt(Dot(v, v));
}

inline Vec3 MoveDirXZ(Vec3 value)
{
	Vec3 zeroY = NewV3(value.x, 0.0f, value.z);
	return Normalize(zeroY);
}

struct Vec3i
{
	int x, y, z;
};

inline Vec3i NewV3i(int x, int y, int z)
{
	return { x, y, z };
}

inline Vec3i operator -(Vec3i a, Vec3i b)
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vec3i BlockPos(Vec3 pos)
{
	return NewV3i(RoundToInt(pos.x), RoundToInt(pos.y), RoundToInt(pos.z));
}

inline Vec3 GetV3(Vec3i v)
{
	return NewV3((float)v.x, (float)v.y, (float)v.z);
}

union Vec4
{
	struct
	{
		float x, y, z, w;
	};

	struct 
	{
		float4 data;
	};

	inline float& operator [](int i)
	{
		return (&x)[i];
	}
};

inline Vec4 NewV4(float v)
{
	return { { v, v, v, v } };
}

inline Vec4 NewV4(float x, float y, float z, float w)
{
	return { { x, y, z, w } };
}

inline Vec4 NewV4(Vec3 v, float w)
{
	return { { v.x, v.y, v.z, w } };
}

inline Vec4 operator *(Vec4 a, Vec4 b)
{
	Vec4 result;
	result.data = Mul_4x(a.data, b.data);
	return result;
}

inline Vec4 operator *(Vec4 v, float s)
{
	Vec4 result;
	float4 s4 = Set1_4x(s);
	result.data = Mul_4x(v.data, s4);
	return result;
}

inline Vec4 operator /(Vec4 v, float s)
{
	Vec4 result;
	float4 s4 = Set1_4x(s);
	result.data = Div_4x(v.data, s4);
	return result;
}

inline Vec4 operator +(Vec4 a, Vec4 b)
{
	Vec4 result;
	result.data = Add_4x(a.data, b.data);
	return result;
}

inline Vec4 operator -(Vec4 v, float s)
{
	Vec4 result;
	float4 s4 = Set1_4x(s);
	result.data = Sub_4x(v.data, s4);
	return result;
}

struct Ray
{
	Vec3 origin;
	Vec3 dir;
};
