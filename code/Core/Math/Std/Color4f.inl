/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Color4f.h"

namespace traktor
{

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

T_MATH_INLINE Scalar Color4f::getEV() const
{
	float ev = 0.0f;
	float mx = 1.0f;
	float mg = m_data.xyz0().max();
	while (mg > mx)
	{
		ev += 1.0f;
		mx = std::pow(2.0f, ev + 1.0f);
	}
	return Scalar(ev);
}

T_MATH_INLINE void Color4f::setEV(const Scalar& ev)
{
	const Scalar ev0 = getEV();

	const float mn0 = ev0 > 0.0f ? std::pow(2.0f, ev0) : 0.0f;
	m_data -= Vector4(mn0, mn0, mn0, 0.0f);

	const float mn = ev > 0.0f ? std::pow(2.0f, ev) : 0.0f;
	m_data += Vector4(mn, mn, mn, 0.0f);
}

T_MATH_INLINE Color4f Color4f::sRGB() const
{
	return Color4f(
		std::pow(getRed(), 2.2f),
		std::pow(getGreen(), 2.2f),
		std::pow(getBlue(), 2.2f),
		getAlpha()
	);
}

T_MATH_INLINE Color4f Color4f::linear() const
{
	return Color4f(
		std::pow(getRed(), 1.0f / 2.2f),
		std::pow(getGreen(), 1.0f / 2.2f),
		std::pow(getBlue(), 1.0f / 2.2f),
		getAlpha()
	);
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

T_MATH_INLINE Color4f Color4f::aaa0() const
{
	return aaaa().rgb0();
}

T_MATH_INLINE Color4f Color4f::aaa1() const
{
	return aaaa().rgb1();
}

T_MATH_INLINE Color4f Color4f::aaaa() const
{
	return Color4f(m_data.shuffle< 3, 3, 3, 3 >());
}

T_MATH_INLINE Color4ub Color4f::toColor4ub() const
{
#if defined(T_MATH_USE_SSE2)
	__m128 v = m_data.m_data;
	__m128 sv = _mm_mul_ps(v, _mm_set1_ps(255.0f));
	__m128i iv = _mm_cvtps_epi32(sv);
	__m128i piv = _mm_packus_epi32(iv, iv);
	piv = _mm_packus_epi16(piv, piv);
	return Color4ub((uint32_t)_mm_cvtsi128_si32(piv));
#else
	float T_MATH_ALIGN16 tmp[4];
	(*this * Scalar(255.0f)).storeAligned(tmp);
	return Color4ub(
		uint8_t(clamp< int32_t >(int32_t(tmp[0]), 0, 255)),
		uint8_t(clamp< int32_t >(int32_t(tmp[1]), 0, 255)),
		uint8_t(clamp< int32_t >(int32_t(tmp[2]), 0, 255)),
		uint8_t(clamp< int32_t >(int32_t(tmp[3]), 0, 255))
	);
#endif
}

T_MATH_INLINE Color4f Color4f::loadAligned(const float* in)
{
	T_ASSERT(in);
	return Color4f(Vector4::loadAligned(in));
}

T_MATH_INLINE Color4f Color4f::loadUnaligned(const float* in)
{
	T_ASSERT(in);
	return Color4f(Vector4::loadUnaligned(in));
}

T_MATH_INLINE Color4f Color4f::fromColor4ub(const Color4ub& in)
{
	return Color4f(
		float(in.r) / 255.0f,
		float(in.g) / 255.0f,
		float(in.b) / 255.0f,
		float(in.a) / 255.0f
	);
}

T_MATH_INLINE void Color4f::storeAligned(float* out) const
{
	T_ASSERT(out);
	m_data.storeAligned(out);
}

T_MATH_INLINE void Color4f::storeUnaligned(float* out) const
{
	T_ASSERT(out);
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

T_MATH_INLINE Color4f Color4f::operator / (const Color4f& r) const
{
	return Color4f(m_data / r.m_data);
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

T_MATH_INLINE Color4f& Color4f::operator *= (const Color4f& r)
{
	m_data *= r.m_data;
	return *this;
}

T_MATH_INLINE Color4f& Color4f::operator *= (const Scalar& r)
{
	m_data *= r;
	return *this;
}

T_MATH_INLINE Color4f& Color4f::operator /= (const Color4f& r)
{
	m_data /= r.m_data;
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

T_MATH_INLINE Color4f min(const Color4f& l, const Color4f& r)
{
	return Color4f(min((Vector4)l, (Vector4)r));
}

T_MATH_INLINE Color4f max(const Color4f& l, const Color4f& r)
{
	return Color4f(max((Vector4)l, (Vector4)r));
}

T_MATH_INLINE Color4f lerp(const Color4f& a, const Color4f& b, Scalar c)
{
	return Color4f(lerp((Vector4)a, (Vector4)b, c));
}

}
