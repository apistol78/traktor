#include <cmath>
#include "Core/Math/Color4ub.h"

namespace traktor
{

T_MATH_INLINE Color4ub::Color4ub()
{
	r =
	g =
	b = 255;
	a = 255;
}

T_MATH_INLINE Color4ub::Color4ub(const Color4ub& v)
{
	r = v.r;
	g = v.g;
	b = v.b;
	a = v.a;
}

T_MATH_INLINE Color4ub::Color4ub(uint8_t r_, uint8_t g_, uint8_t b_)
{
	this->r = r_;
	this->g = g_;
	this->b = b_;
	this->a = 255;
}

T_MATH_INLINE Color4ub::Color4ub(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
	this->r = r_;
	this->g = g_;
	this->b = b_;
	this->a = a_;
}

T_MATH_INLINE Color4ub::Color4ub(uint32_t argb)
{
	this->r = (argb >> 16) & 255;
	this->g = (argb >> 8) & 255;
	this->b = (argb) & 255;
	this->a = (argb >> 24) & 255;
}

T_MATH_INLINE void Color4ub::set(uint8_t r_, uint8_t g_, uint8_t b_)
{
	this->r = r_;
	this->g = g_;
	this->b = b_;
	this->a = 255;
}

T_MATH_INLINE void Color4ub::set(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
	this->r = r_;
	this->g = g_;
	this->b = b_;
	this->a = a_;
}

T_MATH_INLINE uint32_t Color4ub::getARGB() const
{
	return 
		(uint32_t(a) << 24) |
		(uint32_t(r) << 16) |
		(uint32_t(g) << 8) |
		(uint32_t(b));
}

T_MATH_INLINE uint32_t Color4ub::getABGR() const
{
	return 
		(uint32_t(a) << 24) |
		(uint32_t(b) << 16) |
		(uint32_t(g) << 8) |
		(uint32_t(r));
}

T_MATH_INLINE uint32_t Color4ub::getRGBA() const
{
	return
		(uint32_t(r) << 24) |
		(uint32_t(g) << 16) |
		(uint32_t(b) << 8) |
		(uint32_t(a));
}

T_MATH_INLINE uint32_t Color4ub::getBGRA() const
{
	return
		(uint32_t(b) << 24) |
		(uint32_t(g) << 16) |
		(uint32_t(r) << 8) |
		(uint32_t(a));
}

T_MATH_INLINE void Color4ub::getRGBA32F(float outRGBA[4]) const
{
	outRGBA[0] = float(r) / 255.0f;
	outRGBA[1] = float(g) / 255.0f;
	outRGBA[2] = float(b) / 255.0f;
	outRGBA[3] = float(a) / 255.0f;
}

T_MATH_INLINE uint16_t Color4ub::getRGB565() const
{
	return
		((r & 0xf8) << 8) |
		((g & 0xfc) << 3) |
		((b >> 3));
}

T_MATH_INLINE Color4ub& Color4ub::operator = (const Color4ub& v)
{
	r = v.r;
	g = v.g;
	b = v.b;
	a = v.a;
	return *this;
}

T_MATH_INLINE Color4ub Color4ub::operator + (const Color4ub& color) const
{
	return Color4ub(
		r + color.r,
		g + color.g,
		b + color.b,
		a + color.a
	);
}

T_MATH_INLINE Color4ub Color4ub::operator * (const Color4ub& color) const
{
	return Color4ub(
		uint32_t(r * color.r) >> 8,
		uint32_t(g * color.g) >> 8,
		uint32_t(b * color.b) >> 8,
		uint32_t(a * color.a) >> 8
	);
}

T_MATH_INLINE bool Color4ub::operator == (const Color4ub& v) const
{
	return r == v.r && g == v.g && b == v.b && a == v.a;
}

T_MATH_INLINE bool Color4ub::operator != (const Color4ub& v) const
{
	return r != v.r || g != v.g || b != v.b || a != v.a;
}

T_MATH_INLINE Color4ub::operator uint32_t () const
{
	return getARGB();
}

T_MATH_INLINE Color4ub lerp(const Color4ub& a, const Color4ub& b, float c)
{
	return Color4ub(
		uint8_t(a.r * (1.0f - c) + b.r * c),
		uint8_t(a.g * (1.0f - c) + b.g * c),
		uint8_t(a.b * (1.0f - c) + b.b * c),
		uint8_t(a.a * (1.0f - c) + b.a * c)
	);
}

}
