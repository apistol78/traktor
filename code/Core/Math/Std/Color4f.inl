#include <cmath>
#include "Core/Math/Color4f.h"

namespace traktor
{

T_MATH_INLINE Color4f::Color4f()
:	m_data(Vector4::zero())
{
}

T_MATH_INLINE Color4f::Color4f(const Color4f& src)
:	m_data(src.m_data)
{
}

T_MATH_INLINE Color4f::Color4f(float red, float green, float blue, float alpha)
:	m_data(red, green, blue, alpha)
{
}

T_MATH_INLINE Color4f::Color4f(const Vector4& data)
:	m_data(data)
{
}

T_MATH_INLINE Scalar Color4f::get(int channel) const
{
	return m_data[channel];
}

T_MATH_INLINE Scalar Color4f::getRed() const
{
	return m_data.x();
}

T_MATH_INLINE Scalar Color4f::getGreen() const
{
	return m_data.y();
}

T_MATH_INLINE Scalar Color4f::getBlue() const
{
	return m_data.z();
}

T_MATH_INLINE Scalar Color4f::getAlpha() const
{
	return m_data.w();
}

T_MATH_INLINE void Color4f::set(int channel, const Scalar& value)
{
	m_data.set(channel, value);
}

T_MATH_INLINE void Color4f::set(float red, float green, float blue, float alpha)
{
	m_data.set(red, green, blue, alpha);
}

T_MATH_INLINE void Color4f::setRed(const Scalar& red)
{
	m_data.set(0, red);
}

T_MATH_INLINE void Color4f::setGreen(const Scalar& green)
{
	m_data.set(1, green);
}

T_MATH_INLINE void Color4f::setBlue(const Scalar& blue)
{
	m_data.set(2, blue);
}

T_MATH_INLINE void Color4f::setAlpha(const Scalar& alpha)
{
	m_data.set(3, alpha);
}

T_MATH_INLINE Color4f& Color4f::operator = (const Color4f& src)
{
	m_data = src.m_data;
	return *this;
}

T_MATH_INLINE Color4f Color4f::operator + (const Color4f& r) const
{
	return Color4f(m_data + r.m_data);
}

T_MATH_INLINE Color4f Color4f::operator - (const Color4f& r) const
{
	return Color4f(m_data - r.m_data);
}

T_MATH_INLINE Color4f Color4f::operator * (const Color4f& r) const
{
	return Color4f(m_data * r.m_data);
}

T_MATH_INLINE Color4f Color4f::operator * (const Scalar& r) const
{
	return Color4f(m_data * r);
}

T_MATH_INLINE Color4f Color4f::operator / (const Scalar& r) const
{
	return Color4f(m_data / r);
}

T_MATH_INLINE Color4f& Color4f::operator += (const Color4f& r)
{
	m_data += r.m_data;
	return *this;
}

T_MATH_INLINE Color4f& Color4f::operator -= (const Color4f& r)
{
	m_data -= r.m_data;
	return *this;
}

T_MATH_INLINE Color4f& Color4f::operator *= (const Scalar& r)
{
	m_data *= r;
	return *this;
}

T_MATH_INLINE Color4f& Color4f::operator /= (const Scalar& r)
{
	m_data /= r;
	return *this;
}

T_MATH_INLINE bool Color4f::operator == (const Color4f& r) const
{
	return bool(m_data == r.m_data);
}

T_MATH_INLINE bool Color4f::operator != (const Color4f& r) const
{
	return bool(m_data != r.m_data);
}

}
