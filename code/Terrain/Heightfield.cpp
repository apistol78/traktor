#include <cmath>
#include "Terrain/Heightfield.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.Heightfield", Heightfield, Object)

Heightfield::Heightfield(const HeightfieldResource& resource)
:	m_resource(resource)
{
	uint32_t size = m_resource.getSize();
	m_heights = new height_t [size * size];
	T_ASSERT_M (m_heights, L"Unable to allocate heights");
}

Heightfield::~Heightfield()
{
	delete[] m_heights;
}

float Heightfield::getGridHeight(float gridX, float gridZ) const
{
	const Vector4& worldExtent = m_resource.getWorldExtent();
	uint32_t size = m_resource.getSize();

	int igridX = int(std::floor(gridX));
	int igridZ = int(std::floor(gridZ));

	if (igridX < 0)
		igridX = 0;
	else if (igridX >= int(size) - 1)
		igridX = size - 2;

	if (igridZ < 0)
		igridZ = 0;
	else if (igridZ >= int(size) - 1)
		igridZ = size - 2;

	height_t hts[] =
	{
		m_heights[igridX + igridZ * size],
		m_heights[igridX + 1 + igridZ * size],
		m_heights[igridX + (igridZ + 1) * size],
		m_heights[igridX + 1 + (igridZ + 1) * size]
	};

	float fgridX = gridX - igridX;
	float fgridZ = gridZ - igridZ;

	float hl = hts[0] * (1.0f - fgridZ) + hts[2] * fgridZ;
	float hr = hts[1] * (1.0f - fgridZ) + hts[3] * fgridZ;

	return (hl * (1.0f - fgridX) + hr * fgridX) / 65535.0f;
}

float Heightfield::getWorldHeight(float worldX, float worldZ) const
{
	const Vector4& worldExtent = m_resource.getWorldExtent();
	uint32_t size = m_resource.getSize();

	float gridX = size * (worldX + worldExtent.x() * 0.5f) / worldExtent.x() - 0.5f;
	float gridZ = size * (worldZ + worldExtent.z() * 0.5f) / worldExtent.z() - 0.5f;
	float gridY = getGridHeight(gridX, gridZ);

	return -worldExtent.y() * 0.5f + gridY * worldExtent.y();
}

const Heightfield::height_t* Heightfield::getHeights() const
{
	return m_heights;
}

void Heightfield::setHeightTexture(render::ITexture* heightTexture)
{
	m_heightTexture = heightTexture;
}

void Heightfield::setNormalTexture(render::ITexture* normalTexture)
{
	m_normalTexture = normalTexture;
}

Ref< render::ITexture > Heightfield::getHeightTexture() const
{
	return m_heightTexture;
}

Ref< render::ITexture > Heightfield::getNormalTexture() const
{
	return m_normalTexture;
}

	}
}
