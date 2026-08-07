/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/ZPrePass.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ZPrePass", ZPrePass, Object)

ZPrePass::ZPrePass(
	const WorldRenderSettings& settings,
	WorldEntityRenderers* entityRenderers)
	: m_settings(settings)
	, m_entityRenderers(entityRenderers)
{
}

void ZPrePass::destroy()
{
	m_entityRenderers = nullptr;
}

render::RGTargetSet ZPrePass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::RGTexture hiZTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"ZPrePass::setup");

	// Add Z pre-pass target set; depth only, shared with target set of
	// following geometry passes so they inherit the depth written here.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 0;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;

	const render::RGTargetSet zprepassTargetSetId = renderGraph.addTransientTargetSet(L"ZPrePass", rgtd, outputTargetSetId, outputTargetSetId);

	// Add Z pre-pass render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"ZPrePass");
	rp->addWeakInput(hiZTextureId);
	for (const auto& attachment : gatheredView.setupAttachments)
		rp->addInput(attachment);

	render::Clear clear;
	clear.mask = render::CfDepth | render::CfStencil;
	clear.depth = 1.0f;
	clear.stencil = 0;
	rp->setOutput(zprepassTargetSetId, clear, render::TfNone, render::TfDepth);

	rp->addBuild([=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const WorldBuildContext wc(m_entityRenderers, renderContext);

		auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(renderContext);
		sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		sharedParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		sharedParams->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());

		// Cull using same HiZ texture as the g-buffer pass so both passes render
		// the exact same set of geometry.
		if (hiZTextureId != render::RGTexture::Invalid)
		{
			auto hiZTexture = renderGraph.getTexture(hiZTextureId);
			sharedParams->setTextureParameter(ShaderParameter::HiZTexture, hiZTexture);
		}

		sharedParams->endParameters(renderContext);

		const WorldRenderPassShared zprepass(
			ShaderTechnique::ZPrePassWrite,
			sharedParams,
			worldRenderView,
			{});

		T_ASSERT(!renderContext->havePendingDraws());

		for (auto it : gatheredView.renderables)
		{
			IEntityRenderer* entityRenderer = it.first;
			const GatherView::Renderable& r = it.second;
			entityRenderer->build(wc, worldRenderView, zprepass, r.objects);
		}
	});

	renderGraph.addPass(rp);
	return zprepassTargetSetId;
}

}
