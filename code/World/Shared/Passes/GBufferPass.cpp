/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/GBufferPass.h"

#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/IEntityRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GBufferPass", GBufferPass, Object)

GBufferPass::GBufferPass(
	const WorldRenderSettings& settings,
	WorldEntityRenderers* entityRenderers)
	: m_settings(settings)
	, m_entityRenderers(entityRenderers)
{
}

render::RGTargetSet GBufferPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	render::handle_t gbufferWriteTechnique,
	render::RenderGraph& renderGraph,
	render::RGTexture hiZTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"GBufferPass::setup");
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 3;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR16G16B16A16F; // (GBufferA) Depth (R), Normal (GBA)
	rgtd.targets[1].colorFormat = render::TfR8G8B8A8;	   // (GBufferB) Albedo (RGB), Specular (A)
	rgtd.targets[1].sRGB = true;
	rgtd.targets[2].colorFormat = render::TfR8G8B8A8; // (GBufferC) Roughness (R), Metalness (G), Specular (B), Decal Response (A)

	auto gbufferTargetSetId = renderGraph.addTransientTargetSet(L"GBuffer", rgtd, outputTargetSetId, outputTargetSetId);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");
	rp->addWeakInput(hiZTextureId);

	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(clearZ, 0.0f, 0.0f, -1.0f);
	clear.colors[1] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	clear.colors[2] = Color4f(0.8f, 0.0f, 0.0f, 0.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;
	rp->setOutput(gbufferTargetSetId, clear, render::TfNone, render::TfAll);

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const WorldBuildContext wc(
			m_entityRenderers,
			renderContext);

		auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(renderContext);
		sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		sharedParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		sharedParams->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());

		if (hiZTextureId != render::RGTexture::Invalid)
		{
			auto hiZTexture = renderGraph.getTexture(hiZTextureId);
			sharedParams->setTextureParameter(ShaderParameter::HiZTexture, hiZTexture);
		}

		if (gatheredView.rtWorldTopLevel != nullptr)
			sharedParams->setAccelerationStructureParameter(ShaderParameter::TLAS, gatheredView.rtWorldTopLevel);

		sharedParams->endParameters(renderContext);

		const WorldRenderPassShared gbufferPass(
			gbufferWriteTechnique,
			sharedParams,
			worldRenderView,
			IWorldRenderPass::First,
			{});

		T_ASSERT(!renderContext->havePendingDraws());

		for (const auto& r : gatheredView.renderables)
			r.renderer->build(wc, worldRenderView, gbufferPass, r.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->build(wc, worldRenderView, gbufferPass);
	});

	renderGraph.addPass(rp);
	return gbufferTargetSetId;
}

}
