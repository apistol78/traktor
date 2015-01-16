#include <limits>
#include "Terrain/EntityRenderer.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/RiverEntity.h"
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

const TypeInfoSet EntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntity >());
	typeSet.insert(&type_of< OceanEntity >());
	typeSet.insert(&type_of< RiverEntity >());
	return typeSet;
}

void EntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
}

void EntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	if (TerrainEntity* terrainEntity = dynamic_type_cast< TerrainEntity* >(entity))
		terrainEntity->render(worldContext, worldRenderView, worldRenderPass, m_terrainDetailDistance, m_terrainCacheSize, m_terrainLayersEnable);
	else if (OceanEntity* oceanEntity = dynamic_type_cast< OceanEntity* >(entity))
		oceanEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass, m_oceanReflectionEnable);
	else if (RiverEntity* riverEntity = dynamic_type_cast< RiverEntity* >(entity))
		riverEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
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
