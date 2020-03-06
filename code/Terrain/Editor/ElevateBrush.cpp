#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ElevateBrush", ElevateBrush, IBrush)

ElevateBrush::ElevateBrush(const resource::Proxy< hf::Heightfield >& heightfield, drawing::Image* splatImage)
:	m_heightfield(heightfield)
,	m_splatImage(splatImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
,	m_material(-1)
{
}

uint32_t ElevateBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = strength * 0.256f / m_heightfield->getWorldExtent().y();
	m_material = material;
	return MdHeight;
}

void ElevateBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;

			// Check material mask.
			if (m_material >= 0)
			{
				Color4f targetColor;
				m_splatImage->getPixel(x + ix, y + iy, targetColor);

				float T_MATH_ALIGN16 weights[4];
				targetColor.storeAligned(weights);

				a *= 1.0f - weights[m_material];
			}

			if (abs(a) <= FUZZY_EPSILON)
				continue;

			float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, h + a);
		}
	}
}

void ElevateBrush::end(int32_t x, int32_t y)
{
}

Ref< IBrush > ElevateBrush::clone() const
{
	return new ElevateBrush(m_heightfield, m_splatImage);
}

	}
}
