#pragma once

#include "World/IEntityRenderer.h"

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

/*! Terrain entity renderer.
 * \ingroup Terrain
 */
class T_DLLCLASS EntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EntityRenderer(float terrainDetailDistance, uint32_t terrainCacheSize, bool terrainLayersEnable, bool oceanReflectionEnable);

	void setTerrainDetailDistance(float terrainDetailDistance);

	void setTerrainCacheSize(uint32_t terrainCacheSize);

	void setTerrainLayersEnable(bool terrainLayersEnable);

	void setOceanDynamicReflectionEnable(bool oceanReflectionEnable);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		world::Entity* rootEntity
	) override final;

private:
	float m_terrainDetailDistance;
	uint32_t m_terrainCacheSize;
	bool m_terrainLayersEnable;
	bool m_oceanReflectionEnable;
};

	}
}

