/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Resource/Proxy.h"

namespace traktor::render
{

class IRenderSystem;
class RenderGraph;
class ScreenRenderer;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

struct GatherView;
struct WorldCreateDesc;
class WorldRenderView;

/*! Post depth pass.
 * \ingroup World
 *
 * Produces the linear view depth used by post processing, such as depth of field.
 * The GBuffer depth is copied and then surfaces which are not part of the GBuffer,
 * such as the ocean, write their depth on top using the "World_PostDepthWrite" technique.
 */
class PostDepthPass : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	void destroy();

	render::RGTargetSet setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		render::RenderGraph& renderGraph,
		render::RGTargetSet gbufferTargetSetId,
		render::RGTargetSet visualTargetSetId,
		render::RGTargetSet outputTargetSetId) const;

private:
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_copyShader;
};

}
