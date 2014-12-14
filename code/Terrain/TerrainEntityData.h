#ifndef traktor_terrain_TerrainEntityData_H
#define traktor_terrain_TerrainEntityData_H

#include "Resource/Id.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class Terrain;

class T_DLLCLASS TerrainEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	TerrainEntityData();

	virtual void serialize(ISerializer& s);

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

	float getPatchLodDistance() const { return m_patchLodDistance; }

	float getPatchLodBias() const { return m_patchLodBias; }

	float getPatchLodExponent() const { return m_patchLodExponent; }

	float getSurfaceLodDistance() const { return m_surfaceLodDistance; }

	float getSurfaceLodBias() const { return m_surfaceLodBias; }

	float getSurfaceLodExponent() const { return m_surfaceLodExponent; }

private:
	resource::Id< Terrain > m_terrain;
	float m_patchLodDistance;
	float m_patchLodBias;
	float m_patchLodExponent;
	float m_surfaceLodDistance;
	float m_surfaceLodBias;
	float m_surfaceLodExponent;
};

	}
}

#endif	// traktor_terrain_TerrainEntityData_H
