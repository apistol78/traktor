#include <cmath>
#include "Core/Math/Color.h"

namespace traktor
{

T_MATH_INLINE Color::Color()
{
	r =
	g =
	b = 255;
	a = 255;
}

T_MATH_INLINE Color::Color(const Color& v)
{
	r = v.r;
	g = v.g;
	b = v.b;
	a = v.a;
}

T_MATH_INLINE Color::Color(uint8_t r, uint8_t g, uint8_t b)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = 255;
}

T_MATH_INLINE Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

T_MATH_INLINE Color::Color(uint32_t argb)
{
	this->r = (argb >> 16) & 255;
	this->g = (argb >> 8) & 255;
	this->b = (argb) & 255;
	this->a = (argb >> 24) & 255;
}

T_MATH_INLINE void Color::set(uint8_t r, uint8_t g, uint8_t b)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = 255;
}

T_MATH_INLINE void Color::set(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

T_MATH_INLINE uint32_t Color::getARGB() const
{
	return 
		(uint32_t(a) << 24) |
		(uint32_t(r) << 16) |
		(uint32_t(g) << 8) |
		(uint32_t(b));
}

T_MATH_INLINE uint32_t Color::getABGR() const
{
	return 
		(uint32_t(a) << 24) |
		(uint32_t(b) << 16) |
		(uint32_t(g) << 8) |
		(uint32_t(r));
}

T_MATH_INLINE uint32_t Color::getRGBA() const
{
	return
		(uint32_t(r) << 24) |
		(uint32_t(g) << 16) |
		(uint32_t(b) << 8) |
		(uint32_t(a));
}

T_MATH_INLINE uint32_t Color::getBGRA() const
{
	return
		(uint32_t(b) << 24) |
		(uint32_t(g) << 16) |
		(uint32_t(r) << 8) |
		(uint32_t(a));
}

T_MATH_INLINE void Color::getRGBA32F(float outRGBA[4]) const
{
	outRGBA[0] = float(r) / 255.0f;
	outRGBA[1] = float(g) / 255.0f;
	outRGBA[2] = float(b) / 255.0f;
	outRGBA[3] = float(a) / 255.0f;
}

T_MATH_INLINE uint16_t Color::getRGB565() const
{
	return
		((r & 0xf8) << 8) |
		((g & 0xfc) << 3) |
		((b >> 3));
}

T_MATH_INLINE Color& Color::operator = (const Color& v)
{
	r = v.r;
	g = v.g;
	b = v.b;
	a = v.a;
	return *this;
}

T_MATH_INLINE Color Color::operator + (const Color& color) const
{
	return Color(
		r + color.r,
		g + color.g,
		b + color.b,
		a + color.a
	);
}

T_MATH_INLINE Color Color::operator * (const Color& color) const
{
	return Color(
		uint32_t(r * color.r) >> 8,
		uint32_t(g * color.g) >> 8,
		uint32_t(b * color.b) >> 8,
		uint32_t(a * color.a) >> 8
	);
}

T_MATH_INLINE bool Color::operator == (const Color& v) const
{
	return r == v.r && g == v.g && b == v.b && a == v.a;
}

T_MATH_INLINE bool Color::operator != (const Color& v) const
{
	return r != v.r || g != v.g || b != v.b || a != v.a;
}

T_MATH_INLINE Color::operator uint32_t () const
{
	return getARGB();
}

T_MATH_INLINE Color lerp(const Color& a, const Color& b, float c)
{
	return Color(
		uint8_t(a.r * (1.0f - c) + b.r * c),
		uint8_t(a.g * (1.0f - c) + b.g * c),
		uint8_t(a.b * (1.0f - c) + b.b * c),
		uint8_t(a.a * (1.0f - c) + b.a * c)
	);
}

}
