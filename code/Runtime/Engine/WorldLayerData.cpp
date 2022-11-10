/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/WorldLayer.h"
#include "Runtime/Engine/WorldLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.WorldLayerData", 0, WorldLayerData, LayerData)

Ref< Layer > WorldLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< scene::Scene > scene;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_scene, scene))
		return nullptr;

	// Create layer instance.
	return new WorldLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		scene
	);
}

void WorldLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);
	s >> resource::Member< scene::Scene >(L"scene", m_scene);
}

	}
}
