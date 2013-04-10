#include "Terrain/Editor/SmoothBrush.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SmoothBrush", SmoothBrush, IBrush)

SmoothBrush::SmoothBrush(const resource::Proxy< hf::Heightfield >& heightfield, int32_t radius)
:	m_heightfield(heightfield)
,	m_radius(radius)
{
}

void SmoothBrush::begin(int32_t x, int32_t y)
{
}

void SmoothBrush::apply(int32_t x, int32_t y)
{
}

void SmoothBrush::end(int32_t x, int32_t y)
{
}

	}
}
