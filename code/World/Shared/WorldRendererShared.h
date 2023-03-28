/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"

// #define T_WORLD_USE_TILE_JOB

namespace traktor
{

class Job;

}

namespace traktor::render
{

class Buffer;
class ImageGraph;
class ImageGraphContext;
class IRenderTargetSet;
class ITexture;
class ScreenRenderer;

}

namespace traktor::world
{

class AmbientOcclusionPass;
class GBufferPass;
class IEntityRenderer;
class LightComponent;
class PostProcessPass;
class ProbeComponent;
class ReflectionsPass;
class VelocityPass;

/*! World renderer shared implementation.
 * \ingroup World
 */
class T_DLLCLASS WorldRendererShared : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override;

	virtual void destroy() override;

    virtual render::ImageGraphContext* getImageGraphContext() const override final;

protected:
	const static int32_t c_maxLightCount = 16;

#pragma pack(1)
	struct LightShaderData
	{
		float typeRangeRadius[4];
		float position[4];
		float direction[4];
		float color[4];
		float viewToLight0[4];
		float viewToLight1[4];
		float viewToLight2[4];
		float viewToLight3[4];
		float atlasTransform[4];
	};
#pragma pack()

#pragma pack(1)
	struct LightIndexShaderData
	{
		int32_t lightIndex[4];
	};
#pragma pack()

#pragma pack(1)
	struct TileShaderData
	{
		int32_t lightOffsetAndCount[4];
	};
#pragma pack()

	WorldRenderSettings m_settings;

	Quality m_shadowsQuality = Quality::Disabled;

	int32_t m_count = 0;

    Ref< render::IRenderTargetSet > m_sharedDepthStencil;

	Ref< GBufferPass > m_gbufferPass;
	Ref< VelocityPass > m_velocityPass;
	Ref< AmbientOcclusionPass > m_ambientOcclusionPass;
	Ref< ReflectionsPass > m_reflectionsPass;
	Ref< PostProcessPass > m_postProcessPass;

    Ref< render::ImageGraphContext > m_imageGraphContext;

    Ref< WorldEntityRenderers > m_entityRenderers;
    Ref< render::ScreenRenderer > m_screenRenderer;

	Ref< render::ITexture > m_blackTexture;
	Ref< render::ITexture > m_whiteTexture;
	Ref< render::ITexture > m_blackCubeTexture;

	Ref< render::Buffer > m_lightSBuffer;
	Ref< render::Buffer > m_lightIndexSBuffer;
	Ref< render::Buffer > m_tileSBuffer;

	GatherView m_gatheredView;

#if defined(T_WORLD_USE_TILE_JOB)
	Ref< Job > m_tileJob;
#endif

	void setupTileDataPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	);
};

}
