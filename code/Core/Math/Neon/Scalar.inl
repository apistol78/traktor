#include "Core/Math/Scalar.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace
	{

float32x4_t s_vec_div(float32x4_t divend, float32x4_t denom)
{
	const float32x4_t one = { 1.0f, 1.0f, 1.0f, 1.0f };

	float32x4_t idenom = vrecpeq_f32(denom);
	float32x4_t res0 = vmulq_f32(divend, idenom);
	
	float32x4_t t0 = vmulq_f32(denom, idenom);
	float32x4_t t1 = vsubq_f32(t0, one);
	float32x4_t t2 = vnegq_f32(t1);
	
	// r0 + r0 * t2
	return vmlaq_f32(
		res0,
		res0,
		t2
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
	m_data = (float32x4_t){ value, value, value, value };
}

T_MATH_INLINE Scalar::Scalar(float32x4_t value)
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
	return Scalar(vnegq_f32(m_data));
}

T_MATH_INLINE Scalar& Scalar::operator += (const Scalar& v)
{
	m_data = vaddq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator -= (const Scalar& v)
{
	m_data = vsubq_f32(m_data, v.m_data);
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator *= (const Scalar& v)
{
	m_data = vmulq_f32(m_data, v.m_data);
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
	return vgetq_lane_f32(m_data, 0);
}

T_MATH_INLINE Scalar operator + (const Scalar& l, const Scalar& r)
{
	return Scalar(vaddq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator - (const Scalar& l, const Scalar& r)
{
	return Scalar(vsubq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator * (const Scalar& l, const Scalar& r)
{
	return Scalar(vmulq_f32(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar operator / (const Scalar& l, const Scalar& r)
{
	return Scalar(s_vec_div(l.m_data, r.m_data));
}

T_MATH_INLINE Scalar abs(const Scalar& s)
{
	return Scalar(vabsq_f32(s.m_data));
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
	const float32x4_t half = { 0.5f, 0.5f, 0.5f, 0.5f };
	const float32x4_t one = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	float32x4_t estimate, estimateSquared, halfEstimate;

	estimate = vrsqrteq_f32(s.m_data); 

	estimateSquared = vmulq_f32(estimate, estimate);
	halfEstimate = vmulq_f32(estimate, half);
	
	float32x4_t t0 = vmulq_f32(s.m_data, estimateSquared);
	float32x4_t t1 = vsubq_f32(t0, one);
	float32x4_t t2 = vnegq_f32(t1);
	
	estimate = vmlaq_f32(estimate, t2, halfEstimate);

	return Scalar(estimate);
}

T_MATH_INLINE Scalar lerp(const Scalar& a, const Scalar& b, const Scalar& c)
{
	return a * (Scalar(1.0f) - c) + b * c;
}

}
