#include <numeric>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const int32_t c_others[4][3] =
{
	{ 1, 2, 3 },
	{ 0, 2, 3 },
	{ 0, 1, 3 },
	{ 0, 1, 2 }
};

		}

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
	float T_MATH_ALIGN16 weights[4];
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

			targetColor.storeAligned(weights);

			float w = clamp(weights[m_material] + a, 0.0f, 1.0f);

			weights[m_material] = w;

			float s[3], st = 0.0f;
			for (int32_t i = 0; i < 3; ++i)
			{
				float& other = weights[c_others[m_material][i]];
				st += (s[i] = other);
			}
			if (st > FUZZY_EPSILON)
			{
				for (int32_t i = 0; i < 3; ++i)
				{
					float& other = weights[c_others[m_material][i]];
					other = clamp(other, 0.0f, (s[i] / st) * (1.0f - w));
				}
			}

			m_splatImage->setPixel(x + ix, y + iy, Color4f(weights));
		}
	}
}

void MaterialBrush::end(int32_t x, int32_t y)
{
}

Ref< IBrush > MaterialBrush::clone() const
{
	return new MaterialBrush(m_splatImage);
}

	}
}
