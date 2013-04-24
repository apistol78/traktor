#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Terrain/Editor/ColorBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ColorBrush", ColorBrush, IBrush)

ColorBrush::ColorBrush(drawing::Image* colorImage)
:	m_colorImage(colorImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t ColorBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = powf(abs(strength), 2.0f);
	m_color = color;
	return MdColor;
}

void ColorBrush::apply(int32_t x, int32_t y)
{
	Color4f targetColor;
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			int32_t d = ix * ix + iy * iy;
			if (d >= m_radius * m_radius)
				continue;

			float a = m_fallOff->evaluate(1.0f - sqrtf(float(d)) / m_radius) * m_strength;

			m_colorImage->getPixel(x + ix, y + iy, targetColor);
			m_colorImage->setPixel(x + ix, y + iy, (targetColor * Scalar(1.0f - a) + m_color * Scalar(a)).saturated());
		}
	}
}

void ColorBrush::end(int32_t x, int32_t y)
{
}

	}
}
