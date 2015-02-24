#include "Core/Math/Scalar.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace
	{

vec_float4 s_vec_div(vec_float4 divend, vec_float4 denom)
{
	vec_float4 idenom = vec_re(denom);
	vec_float4 res0 = vec_madd(divend, idenom, (vec_float4)(0.0f));
	return vec_madd(
		vec_nmsub(denom, idenom, (vec_float4)(1.0f)),
		res0,
		res0
	);
}

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
	m_data = vec_splats(value);
}

T_MATH_INLINE Scalar::Scalar(vec_float4 value)
:	m_data(value)
{
}

T_MATH_INLINE Scalar& Scalar::operator = (const Scalar& value)
{
	m_data = value.m_data;
	return *this;
}

T_MATH_INLINE Scalar Scalar::operator - () const
{
	vec_uint4 umask = (vec_uint4){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
	return Scalar(vec_xor(m_data, (vec_float4)umask));
}

T_MATH_INLINE Scalar& Scalar::operator += (const Scalar& v)
{
	m_data = vec_add(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator -= (const Scalar& v)
{
	m_data = vec_sub(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator *= (const Scalar& v)
{
	m_data = vec_madd(m_data, v.m_data, (vec_float4)(0.0f));
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator /= (const Scalar& v)
{
	m_data = s_vec_div(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE bool Scalar::operator < (const Scalar& r)
{
	float v1 = float(*this);
	float v2 = float(r);
	return v1 < v2;
}

T_MATH_INLINE bool Scalar::operator <= (const Scalar& r)
{
	float v1 = float(*this);
	float v2 = float(r);
	return v1 <= v2;
}

T_MATH_INLINE bool Scalar::operator > (const Scalar& r)
{
	float v1 = float(*this);
	float v2 = float(r);
	return v1 > v2;
}

T_MATH_INLINE bool Scalar::operator >= (const Scalar& r)
{
	float v1 = float(*this);
	float v2 = float(r);
	return v1 >= v2;
}

T_MATH_INLINE bool Scalar::operator == (const Scalar& r)
{
	float v1 = float(*this);
	float v2 = float(r);
	return v1 == v2;
}

T_MATH_INLINE bool Scalar::operator != (const Scalar& r)
{
	float v1 = float(*this);
	float v2 = float(r);
	return v1 != v2;
}

T_MATH_INLINE Scalar::operator float () const
{
	return vec_extract(m_data, 0);
}

T_MATH_INLINE Scalar operator + (const Scalar& l, const Scalar& r)
{
	return Scalar(vec_add(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator - (const Scalar& l, const Scalar& r)
{
	return Scalar(vec_sub(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator * (const Scalar& l, const Scalar& r)
{
	return Scalar(vec_madd(l.m_data, r.m_data, (vec_float4)(0.0f)));
}

T_MATH_INLINE Scalar operator / (const Scalar& l, const Scalar& r)
{
	return Scalar(s_vec_div(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar abs(const Scalar& s)
{
	vec_uint4 umask = (vec_uint4){ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	return Scalar(vec_and(s.m_data, (vec_float4)umask));
}

T_MATH_INLINE Scalar squareRoot(const Scalar& s)
{
#if 0
	float fs = sqrtf(s);
	return Scalar(fs);
#else
	return abs(s) > Scalar(1e-8f) ? Scalar(1.0f) / reciprocalSquareRoot(s) : Scalar(0.0f);
#endif
}

T_MATH_INLINE Scalar reciprocalSquareRoot(const Scalar& s)
{
	const vec_float4 zero = (vec_float4)(0.0f);
	const vec_float4 half = (vec_float4)(0.5f);
	const vec_float4 one = (vec_float4)(1.0f);
	
	vec_float4 estimate, estimateSquared, halfEstimate;

	estimate = vec_rsqrte(s.m_data); 

	estimateSquared = vec_madd(estimate, estimate, zero);
	halfEstimate = vec_madd(estimate, half, zero);
	estimate = vec_madd(vec_nmsub(s.m_data, estimateSquared, one), halfEstimate, estimate);

	return Scalar(estimate);
}

T_MATH_INLINE Scalar lerp(const Scalar& a, const Scalar& b, const Scalar& c)
{
	return a * (Scalar(1.0f) - c) + b * c;
}

}
