/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "World/EntityBuilder.h"
#include "World/EntityData.h"
#include "World/IWorldComponentData.h"
#include "World/World.h"
#include "World/WorldRenderSettings.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneResource", 0, SceneResource, ISerializable)

SceneResource::SceneResource()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
}

Ref< Scene > SceneResource::createScene(const world::IEntityFactory* entityFactory) const
{
	Ref< world::World > world = new world::World();
	Ref< world::EntityBuilder > entityBuilder = new world::EntityBuilder(entityFactory, world);

	// Create world components.
	for (auto worldComponentData : m_worldComponents)
	{
		Ref< world::IWorldComponent > worldComponent = entityBuilder->create(worldComponentData);
		if (!worldComponent)
			return nullptr;

		world->setComponent(worldComponent);
	}

	// Create world entities.
	Ref< world::Entity > rootEntity = entityBuilder->create(m_entityData);
	if (!rootEntity)
		return nullptr;

	return new Scene(
		m_worldRenderSettings,
		world
	);
}

void SceneResource::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

Ref< world::WorldRenderSettings > SceneResource::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

void SceneResource::setWorldComponents(const RefArray< world::IWorldComponentData >& worldComponents)
{
	m_worldComponents = worldComponents;
}

const RefArray< world::IWorldComponentData >& SceneResource::getWorldComponents() const
{
	return m_worldComponents;
}

void SceneResource::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > SceneResource::getEntityData() const
{
	return m_entityData;
}

void SceneResource::serialize(ISerializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> MemberRefArray< world::IWorldComponentData >(L"worldComponents", m_worldComponents);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
}

}
