/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

namespace traktor::render
{

class RenderGraph;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class Entity;
class WorldRenderView;

/*!
 */
class DownScalePass : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager);

	render::handle_t setup(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId) const;

private:
	resource::Proxy< render::Shader > m_downScaleShader;
};

}
