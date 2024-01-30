/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Terrain/EntityRenderer.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainLayerComponent.h"
#include "World/WorldBuildContext.h"

namespace traktor::terrain
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
	typeSet.insert< TerrainLayerComponent >();
	return typeSet;
}

void EntityRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
	if (auto terrainComponent = dynamic_type_cast< TerrainComponent* >(renderable))
		terrainComponent->setup(context, worldRenderView, m_terrainDetailDistance, m_terrainCacheSize);
	else if (auto oceanComponent = dynamic_type_cast< OceanComponent* >(renderable))
		oceanComponent->setup(context, worldRenderView);
}

void EntityRenderer::setup(
	const world::WorldSetupContext& context
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
	if (auto terrainComponent = dynamic_type_cast< TerrainComponent* >(renderable))
		terrainComponent->build(context, worldRenderView, worldRenderPass, m_terrainDetailDistance, m_terrainCacheSize);
	else if (auto terrainLayerComponent = dynamic_type_cast< TerrainLayerComponent* >(renderable))
		terrainLayerComponent->build(context, worldRenderView, worldRenderPass);
	else if (auto oceanComponent = dynamic_type_cast< OceanComponent* >(renderable))
		oceanComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass, m_oceanReflectionEnable);
	else if (auto riverComponent = dynamic_type_cast< RiverComponent* >(renderable))
		riverComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass);
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
