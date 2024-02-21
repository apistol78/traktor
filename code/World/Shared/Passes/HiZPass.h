/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
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
class HiZPass : public Object
{
    T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager);

	void setup(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
        render::handle_t gbufferTargetSetId,
		render::handle_t outputHiZTextureId
	) const;

private:
resource::Proxy< render::Shader > m_hiZBuildShader;
};

}
