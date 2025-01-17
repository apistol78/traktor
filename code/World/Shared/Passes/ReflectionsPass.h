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
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/WorldTypes.h"

namespace traktor::render
{

class Buffer;
class ImageGraph;
class IRenderSystem;
class ITexture;
class RenderGraph;
class ScreenRenderer;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

struct WorldCreateDesc;

class WorldRenderView;

/*!
 */
class ReflectionsPass : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	render::handle_t setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		const render::Buffer* lightSBuffer,
		bool needJitter,
		uint32_t frameCount,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t dbufferTargetSetId,
		render::handle_t visualReadTargetSetId,
		render::handle_t velocityTargetSetId,
		render::handle_t outputTargetSetId) const;

private:
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::ImageGraph > m_probeGlobalReflections;
	resource::Proxy< render::ImageGraph > m_probeLocalReflections;
	resource::Proxy< render::ImageGraph > m_screenReflections;
	Quality m_reflectionsQuality = Quality::Disabled;
};

}
