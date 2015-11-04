#include <limits>
#include "Terrain/EntityRenderer.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/TerrainComponent.h"
#include "World/WorldContext.h"

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
	typeSet.insert(&type_of< OceanComponent >());
	typeSet.insert(&type_of< RiverComponent >());
	typeSet.insert(&type_of< TerrainComponent >());
	return typeSet;
}

void EntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (TerrainComponent* terrainComponent = dynamic_type_cast< TerrainComponent* >(renderable))
		terrainComponent->render(worldContext, worldRenderView, worldRenderPass, m_terrainDetailDistance, m_terrainCacheSize, m_terrainLayersEnable);
	else if (OceanComponent* oceanComponent = dynamic_type_cast< OceanComponent* >(renderable))
		oceanComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass, m_oceanReflectionEnable);
	else if (RiverComponent* riverComponent = dynamic_type_cast< RiverComponent* >(renderable))
		riverComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
}

void EntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
