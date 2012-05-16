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

float Heightfield::getGridHeightNearest(int32_t gridX, int32_t gridZ) const
{
	if (gridX < 0)
		gridX = 0;
	else if (gridX >= int32_t(m_size))
		gridX = int32_t(m_size) - 1;

	if (gridZ < 0)
		gridZ = 0;
	else if (gridZ >= int32_t(m_size))
		gridZ = int32_t(m_size) - 1;

	return m_heights[gridX + gridZ * m_size] / 65535.0f;
}

float Heightfield::getGridHeightBilinear(float gridX, float gridZ) const
{
	int32_t igridX = int32_t(std::floor(gridX));
	int32_t igridZ = int32_t(std::floor(gridZ));

	if (igridX < 0)
		igridX = 0;
	else if (igridX >= int32_t(m_size) - 1)
		igridX = int32_t(m_size) - 2;

	if (igridZ < 0)
		igridZ = 0;
	else if (igridZ >= int32_t(m_size) - 1)
		igridZ = int32_t(m_size) - 2;

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
	float gridY = getGridHeightBilinear(gridX, gridZ);
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

float Heightfield::unitToWorld(float unitY) const
{
	return -m_worldExtent.y() * 0.5f + unitY * m_worldExtent.y();
}

	}
}
