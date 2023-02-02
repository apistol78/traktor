/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Shader.h"
#include "Render/Image2/Compute.h"
#include "Render/Image2/ComputeData.h"
#include "Resource/IResourceManager.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeData", 0, ComputeData, ImagePassOpData)

Ref< const ImagePassOp > ComputeData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* /*renderSystem*/) const
{
	Ref< Compute > instance = new Compute();

	// Bind shader.
	if (!resourceManager->bind(m_shader, instance->m_shader))
		return nullptr;

	// Get handles of sources.
	for (const auto& source : m_sources)
	{
		instance->m_sources.push_back({
			getParameterHandle(source.textureId),
			getParameterHandle(source.parameter)
		});
	}

	return instance; 
}

}
