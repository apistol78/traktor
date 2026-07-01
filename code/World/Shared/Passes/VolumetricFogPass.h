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
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class ITexture;
class ProgramParameters;
class RenderGraph;
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

/*!
 */
class VolumetricFogPass : public Object
{
	T_RTTI_CLASS;

public:
	explicit VolumetricFogPass(const WorldRenderSettings& settings);

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	void destroy();

	render::RGTexture setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		const render::Buffer* lightSBuffer,
		const render::Buffer* tileSBuffer,
		const render::Buffer* lightIndexSBuffer,
		render::ITexture* whiteTexture,
		uint32_t frameCount,
		const float* slicePositions,
		render::RenderGraph& renderGraph,
		render::RGTargetSet shadowMapAtlasTargetSetId) const;

	static void setupSharedParameters(const GatherView& gatheredView, float viewNearZ, float viewFarZ, render::ProgramParameters* parameters);

private:
	WorldRenderSettings m_settings;
	Quality m_shadowsQuality = Quality::Disabled;
	resource::Proxy< render::Shader > m_injectShader;
	Ref< render::ITexture > m_volumeTextures[2];
};

}
