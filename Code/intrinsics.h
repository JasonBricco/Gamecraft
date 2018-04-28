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
}
