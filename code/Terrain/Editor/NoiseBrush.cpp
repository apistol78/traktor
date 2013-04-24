#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/NoiseBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

Random g_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.NoiseBrush", NoiseBrush, IBrush)

NoiseBrush::NoiseBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t NoiseBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = strength / m_heightfield->getWorldExtent().y();
	return MdHeight;
}

void NoiseBrush::apply(int32_t x, int32_t y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			int32_t d = ix * ix + iy * iy;
			if (d >= m_radius * m_radius)
				continue;

			float a = m_fallOff->evaluate(1.0f - sqrtf(float(d)) / m_radius) * m_strength;
			float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, h + (g_random.nextFloat() * 2.0f - 1.0f) * a);
		}
	}
}

void NoiseBrush::end(int32_t x, int32_t y)
{
}

	}
}
