// Voxel Engine
// Jason Bricco

#define float4 __m128
#define float8 __m256
#define int4 __m128i
#define int8 __m256i

#define Set1_4x(v) _mm_set1_ps(v)
#define Set1i_4x(v) _mm_set1_epi32(v)
#define Set1_8x(v) _mm256_set1_ps(v)
#define Set1i_8x(v) _mm256_set1_epi32(v)

#define Set0_8x() _mm256_setzero_ps()
#define Set0i_8x() _mm256_setzero_si256()

// Sets four floating point values putting 'a' as the most 
// significant bit and 'd' as the least significant bit.
#define Set_4x(a, b, c, d) _mm_set_ps(a, b, c, d)

// Cast from a float to an int.
#define CastToInt_4x(v) _mm_castps_si128(v)
#define CastToInt_8x(v) _mm256_castps_si256(v)
#define CastToFloat_4x(v) _mm_castsi128_ps(v)
#define CastToFloat_8x(v) _mm256_castsi256_ps(v)

// Converts a float value to an int value and rounds the result.
#define ConvertToInt_4x(v) _mm_cvtps_epi32(v)
#define ConvertToInt_8x(v) _mm256_cvtps_epi32(v)

// Converts an int value to a float value.
#define ConvertToFloat_4x(v) _mm_cvtepi32_ps(v)
#define ConvertToFloat_8x(v) _mm256_cvtepi32_ps(v)

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
#define Add_8x(a, b) _mm256_add_ps(a, b)
#define Addi_8x(a, b) _mm256_add_epi32(a, b)

#define Sub_4x(a, b) _mm_sub_ps(a, b)
#define Subi_4x(a, b) _mm_sub_epi32(a, b)
#define Sub_8x(a, b) _mm256_sub_ps(a, b)

#define Mul_4x(a, b) _mm_mul_ps(a, b)
#define Mul_8x(a, b) _mm256_mul_ps(a, b)
#define Muli_8x(a, b) _mm256_mullo_epi32(a, b)

#define Div_4x(a, b) _mm_div_ps(a, b)

#define NMulAdd_8x(a, b, c) _mm256_fnmadd_ps(a, b, c)

#define Square_4x(v) _mm_mul_ps(v, v)
#define Sqrt_4x(v) _mm_sqrt_ps(v)

#define Dot_4x(a, b) _mm_dp_ps(a, b, 0xff)

#define Shuffle_4x(a, b, ctrl) _mm_shuffle_ps(a, b, ctrl)

#define Min_4x(v, min) _mm_min_ps(v, min)
#define Max_4x(v, max) _mm_max_ps(v, max)
#define Clamp_4x(v, min, max) _mm_min_ps(_mm_max_ps(value, min), max)

#define Or_4x(a, b) _mm_or_si128(a, b)
#define Or_8x(a, b) _mm256_or_si256(a, b)

#define Xori_8x(a, b) _mm256_xor_si256(a, b)
#define Xor_8x(a, b) _mm256_xor_ps(a, b)

#define And_4x(a, b) _mm_and_si128(a, b)
#define Andi_8x(a, b) _mm256_and_si256(a, b)
#define And_8x(a, b) _mm256_and_ps(a, b)

static int8 g_one_8x = Set1i_8x(0xffffffff);
#define Not_8x(v) _mm256_xor_si256(v, g_one_8x)

// Computes the bitwise AND with the bitwise NOT inversion of b.
#define AndNot_4x(a, b) _mm_andnot_si128(a, b)
#define AndNot_8x(a, b) _mm256_andnot_si256(a, b)

#define ShiftLeft_4x(v, amount) _mm_slli_epi32(v, amount)
#define ShiftLeft_8x(a, b) _mm256_slli_epi32(a, b)
#define ShiftRight_4x(v, amount) _mm_srli_epi32(v, amount)
#define ShiftRight_8x(v, amount) _mm256_srai_epi32(v, amount)

#define CompareGreaterEqual_4x(a, b) _mm_cmpge_ps(a, b)
#define CompareLessEqual_4x(a, b) _mm_cmple_ps(a, b)

#define CompareLessThan_8x(a, b) _mm256_cmpgt_epi32(b, a)
#define CompareGreaterEqual_8x(a, b) _mm256_cmp_ps(a, b, _CMP_GE_OQ)
#define CompareEquali_8x(a, b) _mm256_cmpeq_epi32(a, b)

#define Floor_4x(v) _mm_floor_ps(v)
#define Floor_8x(v) _mm256_floor_ps(v)

#define Round_4x(v) _mm_round_ps(v)

#define LoadInt_4x(loc) _mm_loadu_si128(loc)
#define WriteInt_4x(loc, v) _mm_storeu_si128(loc, v)

#define StoreAligned_8x(p,a) _mm256_store_ps(p,a)
#define LoadAligned_8x(p) _mm256_load_ps(p)

#define Store_8x(p,a) _mm256_storeu_ps(p,a)
#define Load_8x(p) _mm256_loadu_ps(p)

#define LoadFloat_4x(loc) _mm_loadu_ps(loc)
#define WriteFloat_4x(loc, v) _mm_storeu_ps(loc, v)

#define BlendV_8x(a, b, mask) _mm256_blendv_ps(a, b, CastToFloat_8x(mask))

#define Mask_8x(m, a) And_8x(CastToFloat_8x(m), a)
