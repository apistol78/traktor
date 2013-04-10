#include "Core/Math/Const.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/ElevateBrush.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ElevateBrush", ElevateBrush, IBrush)

ElevateBrush::ElevateBrush(const resource::Proxy< hf::Heightfield >& heightfield, int32_t radius)
:	m_heightfield(heightfield)
,	m_radius(radius)
{
}

void ElevateBrush::begin(int32_t x, int32_t y)
{
}

void ElevateBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			int32_t d = ix * ix + iy * iy;
			if (d >= m_radius * m_radius)
				continue;

			float dh = 0.0025f * clamp(sinf((1.0f - sqrtf(float(d)) / m_radius) * PI / 4.0f), 0.0f, 1.0f);

			float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, h + dh);
		}
	}
}

void ElevateBrush::end(int32_t x, int32_t y)
{
}

	}
}
