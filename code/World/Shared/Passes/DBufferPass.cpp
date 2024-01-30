/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/DBufferPass.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DBufferPass", DBufferPass, Object)

DBufferPass::DBufferPass(
    const WorldRenderSettings& settings,
    WorldEntityRenderers* entityRenderers
)
:   m_settings(settings)
,   m_entityRenderers(entityRenderers)
{
}

render::handle_t DBufferPass::setup(
	const WorldRenderView& worldRenderView,
    const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"DBufferPass::setup");
	const float clearZ = m_settings.viewFarZ;

	// Add DBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 3;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR16G16B16A16F;	// Albedo (RGBA)
	rgtd.targets[1].colorFormat = render::TfR16G16B16A16F;	// Roughness (R), Metalness (B), Specular (B)
	rgtd.targets[2].colorFormat = render::TfR16G16B16A16F;	// Normals (RGB)
	auto dbufferTargetSetId = renderGraph.addTransientTargetSet(L"DBuffer", rgtd, outputTargetSetId, outputTargetSetId);

	// Add DBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"DBuffer");

	rp->addInput(gbufferTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	clear.colors[1] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	clear.colors[2] = Color4f(0.5f, 0.5f, 0.0f, 0.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;
	rp->setOutput(dbufferTargetSetId, clear, render::TfAll, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				renderContext
			);

			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);

			const auto& projection = worldRenderView.getProjection();

			const Scalar p11 = projection.get(0, 0);
			const Scalar p22 = projection.get(1, 1);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);

			sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));

			sharedParams->setTextureParameter(s_handleGBufferDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleGBufferMiscMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleGBufferNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleGBufferColorMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleIrradianceMap, gbufferTargetSet->getColorTexture(3));

			sharedParams->endParameters(renderContext);

			const WorldRenderPassShared dbufferPass(
				s_techniqueDBufferWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::None,
				{}
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto r : gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, dbufferPass, r.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, dbufferPass);
		}
	);

	renderGraph.addPass(rp);
	return dbufferTargetSetId;
}

}
