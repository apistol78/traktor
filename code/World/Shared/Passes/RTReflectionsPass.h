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
#include "Render/Frame/RenderGraphTypes.h"
#include "Resource/Proxy.h"

namespace traktor::render
{

class Buffer;
class ImageGraph;
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

class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class RTReflectionsPass : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	render::RGTargetSet setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		const render::Buffer* lightSBuffer,
		bool needJitter,
		uint32_t frameCount,
		render::RenderGraph& renderGraph,
		render::RGTargetSet gbufferTargetSetId,
		render::RGTargetSet velocityTargetSetId,
		render::RGTexture halfResDepthTextureId,
		render::RGTargetSet outputTargetSetId) const;

private:
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_reflectionsComputeShader;
	resource::Proxy< render::ImageGraph > m_reflectionsDenoise;
};

}
