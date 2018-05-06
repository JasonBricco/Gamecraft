// Voxel Engine
// Jason Bricco

#define float4 __m128
#define int4 __m128i

#define Set1_4x(v) _mm_set1_ps(v)

// Sets four floating point values putting 'a' as the most 
// significant bit and 'd' as the least significant bit.
#define Set_4x(a, b, c, d) _mm_set_ps(a, b, c, d)

// Cast from a float4 to an int4.
#define CastFloatToInt_4x(v) _mm_castps_si128(v)

// Converts a float4 value to an int4 value and rounds the result.
#define ConvertFloatToInt_4x(v) _mm_cvtps_epi32(v)

// Converts an int4 value to a float4 value.
#define ConvertIntToFloat_4x(v) _mm_cvtepi32_ps(v)

// Given two float4 values, for example:
// R3 R2 R1 R0
// G3 G2 G1 G0 
// Returns the interleaved value:
// G1 R1 G0 R0 
#define UnpackLo_4x(a, b) _mm_unpacklo_epi32(a, b)

// Like UnpackLo(), but returns:
// G3 R3 G2 R2
#define UnpackHi_4x(a, b) _mm_unpackhi_epi32(a, b)

#define Add_4x(a, b) _mm_add_ps(a, b)
#define Sub_4x(a, b) _mm_sub_ps(a, b)
#define Mul_4x(a, b) _mm_mul_ps(a, b)
#define Div_4x(a, b) _mm_div_ps(a, b)

#define Square_4x(v) _mm_mul_ps(v, v)
#define Sqrt_4x(v) _mm_sqrt_ps(v)

#define Dot_4x(a, b) _mm_dp_ps(a, b, 0xff)

#define Shuffle_4x(a, b, ctrl) _mm_shuffle_ps(a, b, ctrl)

#define Min_4x(v, min) _mm_min_ps(v, min)
#define Max_4x(v, max) _mm_max_ps(v, max)
#define Clamp_4x(v, min, max) _mm_min_ps(_mm_max_ps(value, min), max)

#define Or_4x(a, b) _mm_or_si128(a, b)
#define And_4x(a, b) _mm_and_si128(a, b)

// Computes the bitwise AND with the bitwise NOT inversion of b.
#define AndNot_4x(a, b) _mm_andnot_si128(a, b)

#define ShiftLeft_4x(v, amount) _mm_slli_epi32(v, amount)
#define ShiftRight_4x(v, amount) _mm_srli_epi32(v, amount)

#define CompareGreaterEqual_4x(a, b) _mm_cmpge_ps(a, b)
#define CompareLessEqual_4x(a, b) _mm_cmple_ps(a, b)

#define LoadInt_4x(loc) _mm_loadu_si128(loc)
#define WriteInt_4x(loc, v) _mm_storeu_si128(loc, v)

#define LoadFloat_4x(loc) _mm_loadu_ps(loc)
#define WriteFloat_4x(loc, v) _mm_storeu_ps(loc, v)
