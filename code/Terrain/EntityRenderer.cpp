#include <limits>
#include "Terrain/EntityRenderer.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/TerrainComponent.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.EntityRenderer", EntityRenderer, world::IEntityRenderer)

EntityRenderer::EntityRenderer(float terrainDetailDistance, uint32_t terrainCacheSize, bool terrainLayersEnable, bool oceanReflectionEnable)
:	m_terrainDetailDistance(terrainDetailDistance)
,	m_terrainCacheSize(terrainCacheSize)
,	m_terrainLayersEnable(terrainLayersEnable)
,	m_oceanReflectionEnable(oceanReflectionEnable)
{
}

void EntityRenderer::setTerrainDetailDistance(float terrainDetailDistance)
{
	m_terrainDetailDistance = terrainDetailDistance;
}

void EntityRenderer::setTerrainCacheSize(uint32_t terrainCacheSize)
{
	m_terrainCacheSize = terrainCacheSize;
}

void EntityRenderer::setTerrainLayersEnable(bool terrainLayersEnable)
{
	m_terrainLayersEnable = terrainLayersEnable;
}

void EntityRenderer::setOceanDynamicReflectionEnable(bool oceanReflectionEnable)
{
	m_oceanReflectionEnable = oceanReflectionEnable;
}

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< OceanComponent >();
	typeSet.insert< RiverComponent >();
	typeSet.insert< TerrainComponent >();
	return typeSet;
}

void EntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (TerrainComponent* terrainComponent = dynamic_type_cast< TerrainComponent* >(renderable))
	{
		terrainComponent->build(context, worldRenderView, worldRenderPass, m_terrainDetailDistance, m_terrainCacheSize);
		if (m_terrainLayersEnable)
			terrainComponent->buildLayers(context, worldRenderView, worldRenderPass);
	}
	else if (OceanComponent* oceanComponent = dynamic_type_cast< OceanComponent* >(renderable))
		oceanComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass, m_oceanReflectionEnable);
	else if (RiverComponent* riverComponent = dynamic_type_cast< RiverComponent* >(renderable))
		riverComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass);
}

void EntityRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void EntityRenderer::flush(const world::WorldBuildContext& context)
{
}

	}
}
