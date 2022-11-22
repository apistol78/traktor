/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/StaticVector.h"
#include "World/Shared/WorldRendererShared.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;
class Shader;

}

namespace traktor::world
{

class IrradianceGrid;
class WorldEntityRenderers;

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
class T_DLLCLASS WorldRendererDeferred : public WorldRendererShared
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

private:
	WorldRenderSettings::ShadowSettings m_shadowSettings;

	Ref< render::IRenderTargetSet > m_shadowMapCascadeTargetSet;
	Ref< render::IRenderTargetSet > m_shadowMapAtlasTargetSet;

	render::Handle m_handleShadowMapCascade;
	render::Handle m_handleShadowMapAtlas;
	render::Handle m_handleVisual[7];

	resource::Proxy< render::Shader > m_lightShader;
	resource::Proxy< render::Shader > m_fogShader;
	resource::Proxy< render::ImageGraph > m_shadowMaskProject;
	resource::Proxy< IrradianceGrid > m_irradianceGrid;

	float m_slicePositions[MaxSliceCount + 1];
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;

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

	render::handle_t setupShadowMaskPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t shadowMapCascadeTargetSetId,
		int32_t lightCascadeIndex
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
};

}
