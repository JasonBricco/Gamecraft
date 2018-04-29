// Voxel Engine
// Jason Bricco

#define float4 __m128
#define int4 __m128i

namespace SIMD
{
	__forceinline float4 Set(float value)
	{
		return _mm_set1_ps(value);
	}

	// Sets four floating point values putting 'a' as the most 
	// significant bit and 'd' as the least significant bit.
	__forceinline float4 Set(float a, float b, float c, float d)
	{
		return _mm_set_ps(a, b, c, d);
	}

	// Cast from a float4 to an int4.
	__forceinline int4 Cast(float4 value)
	{
		return _mm_castps_si128(value);
	}

	// Converts a float4 value to an int4 value.
	__forceinline int4 Convert(float4 value)
	{
		return _mm_cvttps_epi32(value);
	}

	// Given two float4 values, for example:
	// R3 R2 R1 R0
	// G3 G2 G1 G0 
	// Returns the interleaved value:
	// G1 R1 G0 R0 
	__forceinline int4 UnpackLo(int4 a, int4 b)
	{
		return _mm_unpacklo_epi32(a, b);
	}

	// Like UnpackLo(), but returns:
	// G3 R3 G2 R2
	__forceinline int4 UnpackHi(int4 a, int4 b)
	{
		return _mm_unpackhi_epi32(a, b);
	}

	__forceinline float4 Add(float4 a, float4 b)
	{
		return _mm_add_ps(a, b);
	}

	__forceinline float4 Subtract(float4 a, float4 b)
	{
		return _mm_sub_ps(a, b);
	}

	__forceinline float4 Multiply(float4 a, float4 b)
	{
		return _mm_mul_ps(a, b);
	}

	__forceinline float4 Square(float4 value)
	{
		return _mm_mul_ps(value, value);
	}

	__forceinline float4 Sqrt(float4 value)
	{
		return _mm_sqrt_ps(value);
	}

	__forceinline float4 Dot(float4 x0, float4 x1, float4 y0, float4 y1)
	{
		return _mm_add_ps(_mm_mul_ps(x0, x1), _mm_mul_ps(y0, y1));
	}
	
	__forceinline float4 Min(float4 value, float4 min)
	{
		return _mm_min_ps(value, min);
	}

	__forceinline float4 Max(float4 value, float4 max)
	{
		return _mm_max_ps(value, max);
	}

	__forceinline float4 Clamp(float4 value, float4 min, float4 max)
	{
		return _mm_min_ps(_mm_max_ps(value, min), max);
	}

	__forceinline float4 Or(float4 a, float4 b)
	{
		return _mm_or_ps(a, b);
	}

	__forceinline int4 Or(int4 a, int4 b)
	{
		return _mm_or_si128(a, b);
	}

	__forceinline int4 ShiftLeft(int4 a, int amount)
	{
		return _mm_slli_epi32(a, amount);
	}

	__forceinline int4 WriteUnaligned(int4* loc, int4 value)
	{
		_mm_storeu_si128(loc, value);
	}
}
