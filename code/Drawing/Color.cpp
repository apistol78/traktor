#include "Drawing/Color.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Color", Color, Object)

Color::Color() :
	m_red(0),
	m_green(0),
	m_blue(0),
	m_alpha(0)
{
}

Color::Color(const Color& src) :
	m_red(src.m_red),
	m_green(src.m_green),
	m_blue(src.m_blue),
	m_alpha(src.m_alpha)
{
}

Color::Color(float red, float green, float blue, float alpha) :
	m_red(red),
	m_green(green),
	m_blue(blue),
	m_alpha(alpha)
{
}

float Color::get(int channel) const
{
	return m_channel[channel];
}

float Color::getRed() const
{
	return m_red;
}

float Color::getGreen() const
{
	return m_green;
}

float Color::getBlue() const
{
	return m_blue;
}

float Color::getAlpha() const
{
	return m_alpha;
}

void Color::set(int channel, float value)
{
	m_channel[channel] = value;
}

void Color::set(float red, float green, float blue, float alpha)
{
	m_red = red;
	m_green = green; 
	m_blue = blue;
	m_alpha = alpha;
}

void Color::setRed(float red)
{
	m_red = red;
}

void Color::setGreen(float green)
{
	m_green = green;
}

void Color::setBlue(float blue)
{
	m_blue = blue;
}

void Color::setAlpha(float alpha)
{
	m_alpha = alpha;
}

Color& Color::operator = (const Color& src)
{
	m_red = src.m_red;
	m_green = src.m_green;
	m_blue = src.m_blue;
	m_alpha = src.m_alpha;
	return *this;
}

Color Color::operator + (const Color& r) const
{
	return Color(
		m_red + r.m_red,
		m_green + r.m_green,
		m_blue + r.m_blue,
		m_alpha + r.m_alpha
	);
}

Color Color::operator - (const Color& r) const
{
	return Color(
		m_red - r.m_red,
		m_green - r.m_green,
		m_blue - r.m_blue,
		m_alpha - r.m_alpha
	);
}

Color Color::operator * (const Color& r) const
{
	return Color(
		m_red * r.m_red,
		m_green * r.m_green,
		m_blue * r.m_blue,
		m_alpha * r.m_alpha
	);
}

Color Color::operator * (float r) const
{
	return Color(
		m_red * r,
		m_green * r,
		m_blue * r,
		m_alpha * r
	);
}

Color Color::operator / (float r) const
{
	return Color(
		m_red / r,
		m_green / r,
		m_blue / r,
		m_alpha / r
	);
}

Color& Color::operator += (const Color& r)
{
	m_red += r.m_red;
	m_green += r.m_green;
	m_blue += r.m_blue;
	m_alpha += r.m_alpha;
	return *this;
}

Color& Color::operator -= (const Color& r)
{
	m_red -= r.m_red;
	m_green -= r.m_green;
	m_blue -= r.m_blue;
	m_alpha -= r.m_alpha;
	return *this;
}

Color& Color::operator *= (float r)
{
	m_red *= r;
	m_green *= r;
	m_blue *= r;
	m_alpha *= r;
	return *this;
}

Color& Color::operator /= (float r)
{
	m_red /= r;
	m_green /= r;
	m_blue /= r;
	m_alpha /= r;
	return *this;
}

bool Color::operator == (const Color& r) const
{
	return bool(m_red == r.m_red && m_green == r.m_green && m_blue == r.m_blue && m_alpha == r.m_alpha);
}

bool Color::operator != (const Color& r) const
{
	return bool(m_red != r.m_red || m_green != r.m_green || m_blue != r.m_blue || m_alpha != r.m_alpha);
}

	}
}
