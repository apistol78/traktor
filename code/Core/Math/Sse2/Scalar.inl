#include <cmath>
#include "Core/Math/Scalar.h"

namespace traktor
{
	namespace
	{

#ifdef _MSC_VER
#	define ALIGN16_BEG __declspec(align(16))
#	define ALIGN16_END 
#else
#	define ALIGN16_BEG
#	define ALIGN16_END __attribute__((aligned(16)))
#endif

#define T_PS_CONST(Name, Val) \
	static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define T_PI32_CONST(Name, Val) \
	static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define T_PS_CONST_TYPE(Name, Type, Val) \
	static const ALIGN16_BEG Type _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }

T_PS_CONST(1, 1.0f);
T_PS_CONST(0p5, 0.5f);
T_PS_CONST_TYPE(min_norm_pos, int, 0x00800000);
T_PS_CONST_TYPE(mant_mask, int, 0x7f800000);
T_PS_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);
T_PS_CONST_TYPE(sign_mask, int, (int)0x80000000);
T_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);
T_PI32_CONST(1, 1);
T_PI32_CONST(inv1, ~1);
T_PI32_CONST(2, 2);
T_PI32_CONST(4, 4);
T_PI32_CONST(0x7f, 0x7f);
T_PS_CONST(cephes_SQRTHF, 0.707106781186547524);
T_PS_CONST(cephes_log_p0, 7.0376836292E-2);
T_PS_CONST(cephes_log_p1, -1.1514610310E-1);
T_PS_CONST(cephes_log_p2, 1.1676998740E-1);
T_PS_CONST(cephes_log_p3, -1.2420140846E-1);
T_PS_CONST(cephes_log_p4, +1.4249322787E-1);
T_PS_CONST(cephes_log_p5, -1.6668057665E-1);
T_PS_CONST(cephes_log_p6, +2.0000714765E-1);
T_PS_CONST(cephes_log_p7, -2.4999993993E-1);
T_PS_CONST(cephes_log_p8, +3.3333331174E-1);
T_PS_CONST(cephes_log_q1, -2.12194440e-4);
T_PS_CONST(cephes_log_q2, 0.693359375);
T_PS_CONST(exp_hi,	88.3762626647949f);
T_PS_CONST(exp_lo,	-88.3762626647949f);
T_PS_CONST(cephes_LOG2EF, 1.44269504088896341);
T_PS_CONST(cephes_exp_C1, 0.693359375);
T_PS_CONST(cephes_exp_C2, -2.12194440e-4);
T_PS_CONST(cephes_exp_p0, 1.9875691500E-4);
T_PS_CONST(cephes_exp_p1, 1.3981999507E-3);
T_PS_CONST(cephes_exp_p2, 8.3334519073E-3);
T_PS_CONST(cephes_exp_p3, 4.1665795894E-2);
T_PS_CONST(cephes_exp_p4, 1.6666665459E-1);
T_PS_CONST(cephes_exp_p5, 5.0000001201E-1);

	}

T_MATH_INLINE Scalar::Scalar()
{
}

T_MATH_INLINE Scalar::Scalar(const Scalar& value)
:	m_data(value.m_data)
{
}

T_MATH_INLINE Scalar::Scalar(float value)
{
	m_data = _mm_load1_ps(&value);
}

T_MATH_INLINE Scalar::Scalar(__m128 value)
:	m_data(value)
{
}

T_MATH_INLINE Scalar& Scalar::operator = (const Scalar& v)
{
	m_data = v.m_data;
	return *this;
}

T_MATH_INLINE Scalar Scalar::operator - () const
{
	static const uint32_t T_ALIGN16 c_negateMask[] = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
	__m128 mask = _mm_load_ps((const float *)c_negateMask);
	return Scalar(_mm_xor_ps(m_data, mask));
}

T_MATH_INLINE Scalar& Scalar::operator += (const Scalar& v)
{
	m_data = _mm_add_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator -= (const Scalar& v)
{
	m_data = _mm_sub_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator *= (const Scalar& v)
{
	m_data = _mm_mul_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator /= (const Scalar& v)
{
	m_data = _mm_div_ps(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE bool Scalar::operator < (const Scalar& r)
{
	__m128 tmp = _mm_cmplt_ps(m_data, r.m_data);
	uint32_t imm; _mm_store_ss((float*)&imm, tmp);
	return imm != 0;
}

T_MATH_INLINE bool Scalar::operator <= (const Scalar& r)
{
	__m128 tmp = _mm_cmple_ps(m_data, r.m_data);
	uint32_t imm; _mm_store_ss((float*)&imm, tmp);
	return imm != 0;
}

T_MATH_INLINE bool Scalar::operator > (const Scalar& r)
{
	__m128 tmp = _mm_cmpgt_ps(m_data, r.m_data);
	uint32_t imm; _mm_store_ss((float*)&imm, tmp);
	return imm != 0;
}

T_MATH_INLINE bool Scalar::operator >= (const Scalar& r)
{
	__m128 tmp = _mm_cmpge_ps(m_data, r.m_data);
	uint32_t imm; _mm_store_ss((float*)&imm, tmp);
	return imm != 0;
}

T_MATH_INLINE bool Scalar::operator == (const Scalar& r)
{
	__m128 tmp = _mm_cmpeq_ps(m_data, r.m_data);
	uint32_t imm; _mm_store_ss((float*)&imm, tmp);
	return imm != 0;
}

T_MATH_INLINE bool Scalar::operator != (const Scalar& r)
{
	__m128 tmp = _mm_cmpneq_ps(m_data, r.m_data);
	uint32_t imm; _mm_store_ss((float*)&imm, tmp);
	return imm != 0;
}

T_MATH_INLINE Scalar::operator float () const
{
	float tmp; _mm_store_ss(&tmp, m_data);
	return tmp;
}

T_MATH_INLINE Scalar operator + (const Scalar& l, const Scalar& r)
{
	return Scalar(_mm_add_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator - (const Scalar& l, const Scalar& r)
{
	return Scalar(_mm_sub_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator * (const Scalar& l, const Scalar& r)
{
	return Scalar(_mm_mul_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator / (const Scalar& l, const Scalar& r)
{
	return Scalar(_mm_div_ps(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar abs(const Scalar& s)
{
	static const uint32_t T_ALIGN16 c_absoluteMask[] = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	__m128 mask = _mm_load_ps((const float *)c_absoluteMask);
	return Scalar(_mm_and_ps(s.m_data, mask));
}

T_MATH_INLINE Scalar squareRoot(const Scalar& s)
{
	return Scalar(_mm_sqrt_ps(s.m_data));
}

T_MATH_INLINE Scalar reciprocalSquareRoot(const Scalar& s)
{
	return Scalar(_mm_rsqrt_ps(s.m_data));
}

T_MATH_INLINE Scalar logarithm(const Scalar& s)
{
	__m128 x = s.m_data;

	__m128i emm0;
	__m128 one = *(__m128*)_ps_1;

	__m128 invalid_mask = _mm_cmple_ps(x, _mm_setzero_ps());

	x = _mm_max_ps(x, *(__m128*)_ps_min_norm_pos);

	emm0 = _mm_srli_epi32(_mm_castps_si128(x), 23);

	x = _mm_and_ps(x, *(__m128*)_ps_inv_mant_mask);
	x = _mm_or_ps(x, *(__m128*)_ps_0p5);

	emm0 = _mm_sub_epi32(emm0, *(__m128i*)_pi32_0x7f);
	__m128 e = _mm_cvtepi32_ps(emm0);

	e = _mm_add_ps(e, one);

	__m128 mask = _mm_cmplt_ps(x, *(__m128*)_ps_cephes_SQRTHF);
	__m128 tmp = _mm_and_ps(x, mask);
	x = _mm_sub_ps(x, one);
	e = _mm_sub_ps(e, _mm_and_ps(one, mask));
	x = _mm_add_ps(x, tmp);

	__m128 z = _mm_mul_ps(x,x);

	__m128 y = *(__m128*)_ps_cephes_log_p0;
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p1);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p2);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p3);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p4);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p5);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p6);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p7);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p8);
	y = _mm_mul_ps(y, x);

	y = _mm_mul_ps(y, z);

	tmp = _mm_mul_ps(e, *(__m128*)_ps_cephes_log_q1);
	y = _mm_add_ps(y, tmp);

	tmp = _mm_mul_ps(z, *(__m128*)_ps_0p5);
	y = _mm_sub_ps(y, tmp);

	tmp = _mm_mul_ps(e, *(__m128*)_ps_cephes_log_q2);
	x = _mm_add_ps(x, y);
	x = _mm_add_ps(x, tmp);
	x = _mm_or_ps(x, invalid_mask);

	return Scalar(x);
}

T_MATH_INLINE Scalar exponential(const Scalar& s)
{
	__m128 x = s.m_data;

	__m128 tmp = _mm_setzero_ps(), fx;
	__m128i emm0;
	__m128 one = *(__m128*)_ps_1;

	x = _mm_min_ps(x, *(__m128*)_ps_exp_hi);
	x = _mm_max_ps(x, *(__m128*)_ps_exp_lo);

	fx = _mm_mul_ps(x, *(__m128*)_ps_cephes_LOG2EF);
	fx = _mm_add_ps(fx, *(__m128*)_ps_0p5);

	emm0 = _mm_cvttps_epi32(fx);
	tmp  = _mm_cvtepi32_ps(emm0);

	__m128 mask = _mm_cmpgt_ps(tmp, fx);    
	mask = _mm_and_ps(mask, one);
	fx = _mm_sub_ps(tmp, mask);

	tmp = _mm_mul_ps(fx, *(__m128*)_ps_cephes_exp_C1);
	__m128 z = _mm_mul_ps(fx, *(__m128*)_ps_cephes_exp_C2);
	x = _mm_sub_ps(x, tmp);
	x = _mm_sub_ps(x, z);

	z = _mm_mul_ps(x,x);
  
	__m128 y = *(__m128*)_ps_cephes_exp_p0;
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p1);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p2);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p3);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p4);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p5);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, x);
	y = _mm_add_ps(y, one);

	emm0 = _mm_cvttps_epi32(fx);
	emm0 = _mm_add_epi32(emm0, *(__m128i*)_pi32_0x7f);
	emm0 = _mm_slli_epi32(emm0, 23);
	__m128 pow2n = _mm_castsi128_ps(emm0);

	y = _mm_mul_ps(y, pow2n);

	return Scalar(y);
}

T_MATH_INLINE Scalar power(const Scalar& base, const Scalar& exp)
{
	return exponential(exp * logarithm(base));
}

T_MATH_INLINE Scalar lerp(const Scalar& a, const Scalar& b, const Scalar& c)
{
	return a * (Scalar(1.0f) - c) + b * c;
}

T_MATH_INLINE Scalar clamp(const Scalar& value, const Scalar& minLimit, const Scalar& maxLimit)
{
	if (value < minLimit)
		return minLimit;
	else if (value > maxLimit)
		return maxLimit;
	else
		return value;
}

}
