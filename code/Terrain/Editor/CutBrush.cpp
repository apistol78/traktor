#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/CutBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.CutBrush", CutBrush, IBrush)

CutBrush::CutBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_cut(true)
{
}

uint32_t CutBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color)
{
	m_radius = radius;
	m_cut = (strength < 0.0f);
	return MdCut;
}

void CutBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			int32_t d = ix * ix + iy * iy;
			if (d >= m_radius * m_radius)
				continue;

			m_heightfield->setGridCut(x + ix, y + iy, m_cut);
		}
	}
}

void CutBrush::end(int32_t x, int32_t y)
{
}

	}
}
