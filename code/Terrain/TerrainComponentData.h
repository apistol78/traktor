#pragma once

#include "Core/RefArray.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

class Terrain;

/*! Terrain entity data.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

	float getPatchLodDistance() const { return m_patchLodDistance; }

	float getPatchLodBias() const { return m_patchLodBias; }

	float getPatchLodExponent() const { return m_patchLodExponent; }

	float getSurfaceLodDistance() const { return m_surfaceLodDistance; }

	float getSurfaceLodBias() const { return m_surfaceLodBias; }

	float getSurfaceLodExponent() const { return m_surfaceLodExponent; }

private:
	resource::Id< Terrain > m_terrain;
	float m_patchLodDistance = 100.0f;
	float m_patchLodBias = 0.0f;
	float m_patchLodExponent = 1.0f;
	float m_surfaceLodDistance = 100.0f;
	float m_surfaceLodBias = 0.0f;
	float m_surfaceLodExponent = 1.0f;
};

}
