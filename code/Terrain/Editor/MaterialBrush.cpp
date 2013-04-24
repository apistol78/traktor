#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.MaterialBrush", MaterialBrush, IBrush)

MaterialBrush::MaterialBrush(drawing::Image* splatImage)
:	m_splatImage(splatImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t MaterialBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = powf(abs(strength), 2.0f);
	m_material = material;
	return MdSplat;
}

void MaterialBrush::apply(int32_t x, int32_t y)
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

			m_splatImage->getPixel(x + ix, y + iy, targetColor);

			Color4f targetWeight(0.0f, 0.0f, 0.0f, 0.0f);
			targetWeight.set(m_material, Scalar(a));

			targetColor += targetWeight;
			targetColor = Color4f(((const Vector4&)targetColor).normalized());

			m_splatImage->setPixel(x + ix, y + iy, targetColor);


			//Scalar targetWeight = targetColor.get(m_material);

			//Scalar weight = (targetWeight * Scalar(1.0f - a) + m_color * Scalar(a))

			//m_splatImage->setPixel(x + ix, y + iy, (targetColor * Scalar(1.0f - a) + m_color * Scalar(a)).saturated());
		}
	}
}

void MaterialBrush::end(int32_t x, int32_t y)
{
}

	}
}
