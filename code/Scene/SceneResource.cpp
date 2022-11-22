/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "World/EntityData.h"
#include "World/EntitySchema.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/IEntityFactory.h"
#include "World/WorldRenderSettings.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneResource", 0, SceneResource, ISerializable)

SceneResource::SceneResource()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
}

Ref< Scene > SceneResource::createScene(world::IEntityBuilder* entityBuilder) const
{
	Ref< world::EntitySchema > entitySchema = new world::EntitySchema();
	SmallMap< Guid, Ref< world::Entity > > entityProducts;

	Ref< world::EntityBuilderWithSchema > entityBuilderSchema = new world::EntityBuilderWithSchema(entityBuilder, entitySchema, entityProducts);
	Ref< world::Entity > rootEntity = entityBuilderSchema->create(m_entityData);

	Ref< ISceneController > controller;
	if (m_controllerData)
	{
		controller = m_controllerData->createController(entityProducts, false);
		if (!controller)
		{
			log::error << L"Failed to create scene; unable to instantiate scene controller" << Endl;
			return nullptr;
		}
	}

	return new Scene(
		controller,
		rootEntity,
		m_worldRenderSettings
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

void SceneResource::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > SceneResource::getEntityData() const
{
	return m_entityData;
}

void SceneResource::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneResource::getControllerData() const
{
	return m_controllerData;
}

void SceneResource::serialize(ISerializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
}

}
