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

T_MATH_INLINE Color4f::Color4f(const float* data)
:	m_data(data)
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

T_MATH_INLINE Color4f Color4f::saturated() const
{
	return Color4f(traktor::max(traktor::min(m_data, Vector4(1.0f, 1.0f, 1.0f, 1.0f)), Vector4(0.0f, 0.0f, 0.0f, 0.0f)));
}

T_MATH_INLINE Color4f Color4f::rgb0() const
{
	return Color4f(m_data.xyz0());
}

T_MATH_INLINE Color4f Color4f::rgb1() const
{
	return Color4f(m_data.xyz1());
}

T_MATH_INLINE Color4f Color4f::loadAligned(const float* in)
{
	T_ASSERT (in);
	return Color4f(Vector4::loadAligned(in));
}

T_MATH_INLINE Color4f Color4f::loadUnaligned(const float* in)
{
	T_ASSERT (in);
	return Color4f(Vector4::loadUnaligned(in));
}

T_MATH_INLINE void Color4f::storeAligned(float* out) const
{
	T_ASSERT (out);
	m_data.storeAligned(out);
}

T_MATH_INLINE void Color4f::storeUnaligned(float* out) const
{
	T_ASSERT (out);
	m_data.storeUnaligned(out);
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

T_MATH_INLINE Color4f::operator const Vector4& () const
{
	return m_data;
}

}
