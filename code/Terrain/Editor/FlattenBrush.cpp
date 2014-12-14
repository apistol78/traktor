#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.FlattenBrush", FlattenBrush, IBrush)

FlattenBrush::FlattenBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
,	m_height(0.0f)
{
}

uint32_t FlattenBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = abs(strength) * 0.5f;
	m_height = m_heightfield->getGridHeightNearest(x, y);
	return MdHeight;
}

void FlattenBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, lerp(h, m_height, a));
		}
	}
}

void FlattenBrush::end(int32_t x, int32_t y)
{
}

Ref< IBrush > FlattenBrush::clone() const
{
	return new FlattenBrush(m_heightfield);
}

	}
}
