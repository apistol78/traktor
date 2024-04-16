/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/EntityData.h"
#include "World/IWorldComponentData.h"
#include "World/WorldRenderSettings.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.SceneAsset", 10, SceneAsset, ISerializable)

SceneAsset::SceneAsset()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
}

void SceneAsset::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

Ref< world::WorldRenderSettings > SceneAsset::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

void SceneAsset::setWorldComponents(const RefArray< world::IWorldComponentData >& worldComponents)
{
	m_worldComponents = worldComponents;
}

const RefArray< world::IWorldComponentData >& SceneAsset::getWorldComponents() const
{
	return m_worldComponents;
}

void SceneAsset::setLayers(const RefArray< world::EntityData >& layers)
{
	m_layers = layers;
}

const RefArray< world::EntityData >& SceneAsset::getLayers() const
{
	return m_layers;
}

void SceneAsset::setOperationData(const RefArray< ISerializable >& operationData)
{
	m_operationData = operationData;
}

const RefArray< ISerializable >& SceneAsset::getOperationData() const
{
	return m_operationData;
}

void SceneAsset::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion< SceneAsset >() >= 7);

	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);

	if (s.getVersion< SceneAsset >() >= 10)
		s >> MemberRefArray< world::IWorldComponentData >(L"worldComponents", m_worldComponents);

	if (s.getVersion< SceneAsset >() < 9)
	{
		SmallMap< std::wstring, resource::Id< render::ITexture > > imageProcessParams;
		s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"imageProcessParams", imageProcessParams);
	}

	s >> MemberRefArray< world::EntityData >(L"layers", m_layers, AttributePrivate());

	if (s.getVersion< SceneAsset >() < 10)
	{
		Ref< ISceneControllerData > controllerData;
		s >> MemberRef< ISceneControllerData >(L"controllerData", controllerData);
	}

	if (s.getVersion< SceneAsset >() >= 8)
		s >> MemberRefArray< ISerializable >(L"operationData", m_operationData);
}

}
