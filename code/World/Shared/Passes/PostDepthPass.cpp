/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/PostDepthPass.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_copyShader(L"{06BE4DF8-8D5D-8246-805E-B2A70B6DDB66}");
const render::Handle c_handleSource(L"Source");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostDepthPass", PostDepthPass, Object)

bool PostDepthPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_copyShader, m_copyShader))
	{
		log::error << L"Unable to create post depth pass; missing copy shader." << Endl;
		return false;
	}

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

void PostDepthPass::destroy()
{
	safeDestroy(m_screenRenderer);
	m_copyShader.clear();
}

render::RGTargetSet PostDepthPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet visualTargetSetId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"PostDepthPass::setup");

	// Single channel, linear view depth; same layout as GBufferA red channel.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR32F;

	// Share depth buffer with visual target so depth test against opaque and transparent geometry is possible.
	const render::RGTargetSet postDepthTargetSetId = renderGraph.addTransientTargetSet(L"Post depth", rgtd, gbufferTargetSetId, gbufferTargetSetId);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Post depth");
	rp->addInput(gbufferTargetSetId);
	rp->addInput(visualTargetSetId);
	for (const auto& attachment : gatheredView.setupAttachments)
		rp->addInput(attachment);
	rp->setOutput(postDepthTargetSetId, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		// Copy GBuffer depth as base.
		{
			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);

			auto params = renderContext->alloc< render::ProgramParameters >();
			params->beginParameters(renderContext);
			params->setTextureParameter(c_handleSource, gbufferTargetSet->getColorTexture(0));
			params->endParameters(renderContext);

			m_screenRenderer->draw(renderContext, m_copyShader, params, L"Post depth; copy");
		}

		// Let non-GBuffer surfaces write their depth.
		{
			const WorldBuildContext wc(nullptr, renderContext);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
			sharedParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
			sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			const WorldRenderPassShared postDepthPass(
				ShaderTechnique::PostDepthWrite,
				sharedParams,
				worldRenderView);

			for (auto it : gatheredView.renderables)
			{
				IEntityRenderer* entityRenderer = it.first;
				const GatherView::Renderable& r = it.second;
				entityRenderer->build(wc, worldRenderView, postDepthPass, r.objects);
			}
		}
	});

	renderGraph.addPass(rp);
	return postDepthTargetSetId;
}

}
