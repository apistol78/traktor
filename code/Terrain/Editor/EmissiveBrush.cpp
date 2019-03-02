#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Terrain/Editor/EmissiveBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.EmissiveBrush", EmissiveBrush, IBrush)

EmissiveBrush::EmissiveBrush(drawing::Image* colorImage)
:	m_colorImage(colorImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t EmissiveBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = sign(strength) * powf(abs(strength), 2.0f);
	return MdColor;
}

void EmissiveBrush::apply(int32_t x, int32_t y)
{
	Color4f targetColor;
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * abs(m_strength);
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			m_colorImage->getPixel(x + ix, y + iy, targetColor);
			targetColor.setAlpha(
				Scalar(targetColor.getAlpha() * (1.0f - a) + (m_strength > 0.0f ? 1.0f : 0.0f) * a)
			);
			m_colorImage->setPixel(x + ix, y + iy, targetColor.saturated());
		}
	}
}

void EmissiveBrush::end(int32_t x, int32_t y)
{
}

Ref< IBrush > EmissiveBrush::clone() const
{
	return new EmissiveBrush(m_colorImage);
}

	}
}
