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
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class ImageGraph;
class IRenderTargetSet;
class ITexture;
class ScreenRenderer;
class Shader;

}

namespace traktor::world
{

class IEntityRenderer;
class IrradianceGrid;
class LightComponent;
class LightRendererDeferred;
class WorldEntityRenderers;

struct LightShaderData;
struct TileShaderData;

/*! World renderer, using deferred rendering method.
 * \ingroup World
 *
 * Advanced deferred rendering path.
 *
 * Operation
 * \TBD
 *
 * Techniques used
 * \TBD
 */
class T_DLLCLASS WorldRendererDeferred : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) override final;

	virtual render::ImageGraphContext* getImageGraphContext() const override final;

private:
	struct Gather
	{
		IEntityRenderer* entityRenderer;
		Object* renderable;
	};

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;

	Quality m_toneMapQuality = Quality::Disabled;
	Quality m_motionBlurQuality = Quality::Disabled;
	Quality m_shadowsQuality = Quality::Disabled;
	Quality m_reflectionsQuality = Quality::Disabled;
	Quality m_ambientOcclusionQuality = Quality::Disabled;
	Quality m_antiAliasQuality = Quality::Disabled;

	float m_gamma = 1.0f;

	Ref< render::IRenderTargetSet > m_shadowMapCascadeTargetSet;
	Ref< render::IRenderTargetSet > m_shadowMapAtlasTargetSet;
	Ref< render::IRenderTargetSet > m_sharedDepthStencil;

	Ref< render::ScreenRenderer > m_screenRenderer;
	Ref< LightRendererDeferred > m_lightRenderer;

	Ref< render::ITexture > m_blackTexture;
	Ref< render::ITexture > m_whiteTexture;

	render::Handle m_handleShadowMapCascade;
	render::Handle m_handleShadowMapAtlas;
	render::Handle m_handleVisual[7];

	resource::Proxy< render::Shader > m_lightShader;
	resource::Proxy< render::Shader > m_fogShader;

	resource::Proxy< render::ImageGraph > m_ambientOcclusion;
	resource::Proxy< render::ImageGraph > m_antiAlias;
	resource::Proxy< render::ImageGraph > m_visual;
	resource::Proxy< render::ImageGraph > m_gammaCorrection;
	resource::Proxy< render::ImageGraph > m_velocityPrime;
	resource::Proxy< render::ImageGraph > m_motionBlur;
	resource::Proxy< render::ImageGraph > m_toneMap;
	resource::Proxy< render::ImageGraph > m_screenReflections;
	resource::Proxy< render::ImageGraph > m_shadowMaskProject;

	resource::Proxy< IrradianceGrid > m_irradianceGrid;
	
	Ref< WorldEntityRenderers > m_entityRenderers;
	AlignedVector< Gather > m_gathered;
	AlignedVector< const LightComponent* > m_lights;
	Ref< render::Buffer > m_lightSBuffer;
	void* m_lightSBufferMemory = nullptr;
	Ref< render::Buffer > m_tileSBuffer;
	void* m_tileSBufferMemory = nullptr;

	float m_slicePositions[MaxSliceCount + 1];
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;

	int32_t m_count = 0;

	render::handle_t setupGBufferPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) const;

	render::handle_t setupVelocityPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId
	) const;

	render::handle_t setupAmbientOcclusionPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId
	) const;

	render::handle_t setupCascadeShadowMapPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		int32_t lightCascadeIndex,
		LightShaderData* lightShaderData
	) const;

	render::handle_t setupAtlasShadowMapPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		const StaticVector< int32_t, 16 >& lightAtlasIndices,
		LightShaderData* lightShaderData
	) const;

	void setupTileDataPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		TileShaderData* tileShaderData
	) const;

	render::handle_t setupShadowMaskPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t shadowMapCascadeTargetSetId,
		int32_t lightCascadeIndex
	) const;

	render::handle_t setupReflectionsPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t visualReadTargetSetId
	) const;

	void setupVisualPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t visualWriteTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t ambientOcclusionTargetSetId,
		render::handle_t reflectionsTargetSetId,
		render::handle_t shadowMaskTargetSetId,
		render::handle_t shadowMapAtlasTargetSetId
	) const;

	void setupProcessPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t velocityTargetSetId,
		render::handle_t visualWriteTargetSetId,
		render::handle_t visualReadTargetSetId
	) const;
};

}
