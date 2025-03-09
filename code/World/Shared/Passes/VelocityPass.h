/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
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

class ImageGraph;
class IRenderSystem;
class RenderGraph;
class ScreenRenderer;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

struct GatherView;
struct WorldCreateDesc;

class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class VelocityPass : public Object
{
	T_RTTI_CLASS;

public:
	explicit VelocityPass(WorldEntityRenderers* entityRenderers);

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	render::RGTargetSet setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		render::RenderGraph& renderGraph,
		render::RGTargetSet gbufferTargetSetId,
		render::RGTargetSet outputTargetSetId) const;

private:
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::ImageGraph > m_velocityPrime;
};

}
