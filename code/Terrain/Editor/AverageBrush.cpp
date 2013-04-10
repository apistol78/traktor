#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/AverageBrush.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.AverageBrush", AverageBrush, IBrush)

AverageBrush::AverageBrush(const resource::Proxy< hf::Heightfield >& heightfield, int32_t radius)
:	m_heightfield(heightfield)
,	m_radius(radius)
{
}

void AverageBrush::begin(int32_t x, int32_t y)
{
}

void AverageBrush::apply(int32_t x, int32_t y)
{
	float height = 0.0f;
	int32_t count = 0;

	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			int32_t d = ix * ix + iy * iy;
			if (d >= m_radius * m_radius)
				continue;

			height += m_heightfield->getGridHeightNearest(x + ix, y + iy);
			count++;
		}
	}

	if (count <= 0)
		return;

	height /= float(count);

	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			int32_t d = ix * ix + iy * iy;
			if (d >= m_radius * m_radius)
				continue;

			float a = clamp(sinf((1.0f - sqrtf(float(d)) / m_radius) * PI / 4.0f), 0.0f, 1.0f);

			float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, lerp(h, height, a));
		}
	}
}

void AverageBrush::end(int32_t x, int32_t y)
{
}

	}
}
