#include "Core/Math/Scalar.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace
	{

vec_float4 s_vec_div(vec_float4 divend, vec_float4 denom)
{
	vec_float4 idenom = spu_re(denom);
	vec_float4 res0 = spu_madd(divend, idenom, (vec_float4)(0.0f));
	return spu_madd(
		spu_nmsub(denom, idenom, (vec_float4)(1.0f)),
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
	m_data = (vec_float4){ value, value, value, value };
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
	return Scalar(spu_xor(m_data, (vec_float4)umask));
}

T_MATH_INLINE Scalar& Scalar::operator += (const Scalar& v)
{
	m_data = spu_add(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator -= (const Scalar& v)
{
	m_data = spu_sub(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator *= (const Scalar& v)
{
	m_data = spu_madd(m_data, v.m_data, (vec_float4)(0.0f));
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
	return spu_extract(m_data, 0);
}

T_MATH_INLINE Scalar operator + (const Scalar& l, const Scalar& r)
{
	return Scalar(spu_add(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator - (const Scalar& l, const Scalar& r)
{
	return Scalar(spu_sub(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator * (const Scalar& l, const Scalar& r)
{
	return Scalar(spu_madd(l.m_data, r.m_data, (vec_float4)(0.0f)));
}

T_MATH_INLINE Scalar operator / (const Scalar& l, const Scalar& r)
{
	return Scalar(s_vec_div(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar abs(const Scalar& s)
{
	vec_uint4 umask = (vec_uint4){ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	return Scalar(spu_and(s.m_data, (vec_float4)umask));
}

T_MATH_INLINE Scalar squareRoot(const Scalar& s)
{
#if 1
	float fs = sqrtf(s);
	return Scalar(fs);
#else
	// @fixme Too inaccurate
	vec_float4 rsqrt = spu_rsqrte(s.m_data);
	return Scalar(spu_re(rsqrt));
#endif
}

T_MATH_INLINE Scalar reciprocalSquareRoot(const Scalar& s)
{
#if 1
	float rfs = 1.0f / sqrtf(s);
	return Scalar(rfs);
#else
	// @fixme Too inaccurate
	return Scalar(spu_rsqrte(s.m_data));
#endif
}

T_MATH_INLINE Scalar logarithm(const Scalar& s)
{
	return Scalar(0.0f);
}

T_MATH_INLINE Scalar exponential(const Scalar& s)
{
	return Scalar(0.0f);
}

T_MATH_INLINE Scalar power(const Scalar& base, const Scalar& exp)
{
	return Scalar(0.0f);
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
