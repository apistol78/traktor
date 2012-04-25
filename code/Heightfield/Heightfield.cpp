#include <cmath>
#include "Heightfield/Heightfield.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.Heightfield", Heightfield, Object)

Heightfield::Heightfield(
	uint32_t size,
	const Vector4& worldExtent
)
:	m_size(size)
,	m_worldExtent(worldExtent)
{
	m_heights.reset(new height_t [m_size * m_size]);
}

float Heightfield::getGridHeight(float gridX, float gridZ) const
{
	int igridX = int(std::floor(gridX));
	int igridZ = int(std::floor(gridZ));

	if (igridX < 0)
		igridX = 0;
	else if (igridX >= int(m_size) - 1)
		igridX = m_size - 2;

	if (igridZ < 0)
		igridZ = 0;
	else if (igridZ >= int(m_size) - 1)
		igridZ = m_size - 2;

	height_t hts[] =
	{
		m_heights[igridX + igridZ * m_size],
		m_heights[igridX + 1 + igridZ * m_size],
		m_heights[igridX + (igridZ + 1) * m_size],
		m_heights[igridX + 1 + (igridZ + 1) * m_size]
	};

	float fgridX = gridX - igridX;
	float fgridZ = gridZ - igridZ;

	float hl = hts[0] * (1.0f - fgridZ) + hts[2] * fgridZ;
	float hr = hts[1] * (1.0f - fgridZ) + hts[3] * fgridZ;

	return (hl * (1.0f - fgridX) + hr * fgridX) / 65535.0f;
}

float Heightfield::getWorldHeight(float worldX, float worldZ) const
{
	float gridX = m_size * (worldX + m_worldExtent.x() * 0.5f) / m_worldExtent.x() - 0.5f;
	float gridZ = m_size * (worldZ + m_worldExtent.z() * 0.5f) / m_worldExtent.z() - 0.5f;
	float gridY = getGridHeight(gridX, gridZ);
	return -m_worldExtent.y() * 0.5f + gridY * m_worldExtent.y();
}

void Heightfield::gridToWorld(float gridX, float gridZ, float& outWorldX, float& outWorldZ) const
{
	outWorldX = m_worldExtent.x() * (gridX / m_size - 0.5f);
	outWorldZ = m_worldExtent.z() * (gridZ / m_size - 0.5f);
}

void Heightfield::worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const
{
	outGridX = m_size * (worldX + m_worldExtent.x() * 0.5f) / m_worldExtent.x() - 0.5f;
	outGridZ = m_size * (worldZ + m_worldExtent.z() * 0.5f) / m_worldExtent.z() - 0.5f;
}

	}
}
