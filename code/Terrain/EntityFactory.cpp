/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/EntityFactory.h"
#include "Terrain/ForestComponent.h"
#include "Terrain/ForestComponentData.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/RiverComponentData.h"
#include "Terrain/RubbleComponent.h"
#include "Terrain/RubbleComponentData.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/UndergrowthComponent.h"
#include "Terrain/UndergrowthComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.EntityFactory", EntityFactory, world::AbstractEntityFactory)

EntityFactory::EntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ForestComponentData >();
	typeSet.insert< OceanComponentData >();
	typeSet.insert< RiverComponentData >();
	typeSet.insert< RubbleComponentData >();
	typeSet.insert< TerrainComponentData >();
	typeSet.insert< UndergrowthComponentData >();
	return typeSet;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto forestComponentData = dynamic_type_cast< const ForestComponentData* >(&entityComponentData))
	{
		Ref< ForestComponent > forestComponent = new ForestComponent();
		if (forestComponent->create(m_resourceManager, m_renderSystem, *forestComponentData))
			return forestComponent;
	}
	else if (auto oceanComponentData = dynamic_type_cast< const OceanComponentData* >(&entityComponentData))
	{
		Ref< OceanComponent > oceanComponent = new OceanComponent();
		if (oceanComponent->create(m_resourceManager, m_renderSystem, *oceanComponentData))
			return oceanComponent;
	}
	else if (auto riverComponentData = dynamic_type_cast< const RiverComponentData* >(&entityComponentData))
	{
		Ref< RiverComponent > riverComponent = new RiverComponent();
		if (riverComponent->create(m_resourceManager, m_renderSystem, *riverComponentData))
			return riverComponent;
	}
	else if (auto rubbleComponentData = dynamic_type_cast< const RubbleComponentData* >(&entityComponentData))
	{
		Ref< RubbleComponent > rubbleComponent = new RubbleComponent();
		if (rubbleComponent->create(m_resourceManager, m_renderSystem, *rubbleComponentData))
			return rubbleComponent;
	}
	else if (auto terrainComponentData = dynamic_type_cast< const TerrainComponentData* >(&entityComponentData))
	{
		Ref< TerrainComponent > terrainComponent = new TerrainComponent(m_resourceManager, m_renderSystem);
		if (terrainComponent->create(*terrainComponentData))
			return terrainComponent;
	}
	else if (auto undergrowthComponentData = dynamic_type_cast< const UndergrowthComponentData* >(&entityComponentData))
	{
		Ref< UndergrowthComponent > undergrowthComponent = new UndergrowthComponent();
		if (undergrowthComponent->create(m_resourceManager, m_renderSystem, *undergrowthComponentData))
			return undergrowthComponent;
	}
	return nullptr;
}

}
