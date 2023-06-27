/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/GBufferPass.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GBufferPass", GBufferPass, Object)

GBufferPass::GBufferPass(
    const WorldRenderSettings& settings,
    WorldEntityRenderers* entityRenderers,
    render::IRenderTargetSet* sharedDepthStencil
)
:   m_settings(settings)
,   m_entityRenderers(entityRenderers)
,   m_sharedDepthStencil(sharedDepthStencil)
{

}

render::handle_t GBufferPass::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
    const GatherView& gatheredView,
	render::handle_t gbufferWriteTechnique,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"GBufferPass::setup");
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 4;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR16G16B16A16F;	// Depth (R), Roughness (G), Metalness (B), Specular (A)
	rgtd.targets[1].colorFormat = render::TfR16G16B16A16F;	// Normals (RGB)
	rgtd.targets[2].colorFormat = render::TfR16G16B16A16F;	// Albedo (RGB)
	rgtd.targets[3].colorFormat = render::TfR16G16B16A16F;	// Irradiance (RGB)
	auto gbufferTargetSetId = renderGraph.addTransientTargetSet(L"GBuffer", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");
	
	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(clearZ, 1.0f, 0.0f, 0.5f);
	clear.colors[1] = Color4f(0.5f, 0.5f, 0.0f, 0.0f);
	clear.colors[2] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	clear.colors[3] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;
	rp->setOutput(gbufferTargetSetId, clear, render::TfNone, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			const WorldRenderPassShared gbufferPass(
				gbufferWriteTechnique,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::First
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto r : gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, gbufferPass, r.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, gbufferPass);
		}
	);

	renderGraph.addPass(rp);
	return gbufferTargetSetId;
}

}
