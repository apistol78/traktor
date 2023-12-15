/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/SplitWorldLayer.h"
#include "Runtime/Engine/SplitWorldLayerData.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"

namespace traktor::runtime
{
	namespace
	{

const resource::Id< render::Shader > c_shaderSplit(L"{2445959C-5BDC-43BB-B2AB-672FCB8F6E35}");

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.SplitWorldLayerData", 0, SplitWorldLayerData, LayerData)

SplitWorldLayerData::SplitWorldLayerData()
:	LayerData(L"world")
{
}

Ref< Layer > SplitWorldLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< render::Shader > shader;
	resource::Proxy< scene::Scene > scene;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(c_shaderSplit, shader))
		return nullptr;
	if (!resourceManager->bind(m_scene, scene))
		return nullptr;

	// Create layer instance.
	return new SplitWorldLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		scene,
		shader
	);
}

void SplitWorldLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);
	s >> resource::Member< scene::Scene >(L"scene", m_scene);
}

}
