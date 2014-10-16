#include "Core/Math/Scalar.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{

T_MATH_INLINE Scalar::Scalar()
{
}

T_MATH_INLINE Scalar::Scalar(const Scalar& value)
:	m_data(value.m_data)
{
}

T_MATH_INLINE Scalar::Scalar(float value)
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
	return Scalar(-m_data);
}

T_MATH_INLINE Scalar& Scalar::operator += (const Scalar& v)
{
	m_data += v.m_data;
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator -= (const Scalar& v)
{
	m_data -= v.m_data;
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator *= (const Scalar& v)
{
	m_data *= v.m_data;
	return *this;
}

T_MATH_INLINE Scalar& Scalar::operator /= (const Scalar& v)
{
	m_data /= v.m_data;
	return *this;
}

T_MATH_INLINE bool Scalar::operator < (const Scalar& r)
{
	return m_data < r.m_data;
}

T_MATH_INLINE bool Scalar::operator <= (const Scalar& r)
{
	return m_data <= r.m_data;
}

T_MATH_INLINE bool Scalar::operator > (const Scalar& r)
{
	return m_data > r.m_data;
}

T_MATH_INLINE bool Scalar::operator >= (const Scalar& r)
{
	return m_data >= r.m_data;
}

T_MATH_INLINE bool Scalar::operator == (const Scalar& r)
{
	return m_data == r.m_data;
}

T_MATH_INLINE bool Scalar::operator != (const Scalar& r)
{
	return m_data != r.m_data;
}

T_MATH_INLINE Scalar::operator float () const
{
	return m_data;
}

T_MATH_INLINE Scalar operator + (const Scalar& l, const Scalar& r)
{
	return Scalar(l.m_data + r.m_data);
}

T_MATH_INLINE Scalar operator - (const Scalar& l, const Scalar& r)
{
	return Scalar(l.m_data - r.m_data);
}

T_MATH_INLINE Scalar operator * (const Scalar& l, const Scalar& r)
{
	return Scalar(l.m_data * r.m_data);
}

T_MATH_INLINE Scalar operator / (const Scalar& l, const Scalar& r)
{
	return Scalar(l.m_data / r.m_data);
}

T_MATH_INLINE Scalar abs(const Scalar& s)
{
	return Scalar((float)std::fabs(s.m_data));
}

T_MATH_INLINE Scalar squareRoot(const Scalar& s)
{
	return Scalar((float)std::sqrt(s.m_data));
}

T_MATH_INLINE Scalar reciprocalSquareRoot(const Scalar& s)
{
	return Scalar(1.0f / (float)std::sqrt(s.m_data));
}

T_MATH_INLINE Scalar lerp(const Scalar& a, const Scalar& b, const Scalar& c)
{
	return Scalar(a.m_data * (1.0f - c.m_data) + b.m_data * c.m_data);
}

}
