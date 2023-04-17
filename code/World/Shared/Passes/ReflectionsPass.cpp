/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/ReflectionsPass.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::ImageGraph > c_screenReflections(L"{2F8EC56A-FD46-DF42-94B5-9DD676B8DD8A}");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ReflectionsPass", ReflectionsPass, Object)

ReflectionsPass::ReflectionsPass(
    const WorldRenderSettings& settings,
    WorldEntityRenderers* entityRenderers,
    render::IRenderTargetSet* sharedDepthStencil
)
:   m_settings(settings)
,   m_entityRenderers(entityRenderers)
,   m_sharedDepthStencil(sharedDepthStencil)
{
}

bool ReflectionsPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	// Create screen reflections processing.
	if (desc.quality.reflections >= Quality::Disabled)
	{
		if (!resourceManager->bind(c_screenReflections, m_screenReflections))
		{
			log::error << L"Unable to create screen space reflections process." << Endl;
			return false;
		}
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	m_reflectionsQuality = desc.quality.reflections;
	return true;
}

render::handle_t ReflectionsPass::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
    const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t visualReadTargetSetId,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"ReflectionsPass::setup");

	if (m_reflectionsQuality == Quality::Disabled)
		return 0;

	// Add reflections target.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.ignoreStencil = true;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;

	switch (m_reflectionsQuality)
	{
	default:
	case Quality::Low:
		rgtd.referenceWidthDenom = 2;	// 50%
		rgtd.referenceHeightDenom = 2;
		break;

	case Quality::Medium:
		rgtd.referenceWidthMul = 2;		// 67%
		rgtd.referenceWidthDenom = 3;
		rgtd.referenceHeightMul = 2;
		rgtd.referenceHeightDenom = 3;
		break;

	case Quality::High:
		rgtd.referenceWidthMul = 4;		// 80%
		rgtd.referenceWidthDenom = 5;
		rgtd.referenceHeightMul = 4;
		rgtd.referenceHeightDenom = 5;
		break;

	case Quality::Ultra:
		rgtd.referenceWidthDenom = 1;	// 100%
		rgtd.referenceHeightDenom = 1;
		break;
	}

	auto reflectionsTargetSetId = renderGraph.addTransientTargetSet(L"Reflections", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add reflections render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Reflections");

	rp->addInput(gbufferTargetSetId);
	rp->addInput(visualReadTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(reflectionsTargetSetId, clear, render::TfNone, render::TfColor);

	// rp->addBuild(
	// 	[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
	// 	{
	// 		WorldBuildContext wc(
	// 			m_entityRenderers,
	// 			rootEntity,
	// 			renderContext
	// 		);

	// 		auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);

	// 		auto sharedParams = renderContext->alloc< render::ProgramParameters >();
	// 		sharedParams->beginParameters(renderContext);
	// 		sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	// 		sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
	// 		sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
	// 		sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	// 		sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
	// 		sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
	// 		sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
	// 		sharedParams->endParameters(renderContext);

	// 		WorldRenderPassShared reflectionsPass(
	// 			s_techniqueReflectionWrite,
	// 			sharedParams,
	// 			worldRenderView,
	// 			IWorldRenderPass::PfNone
	// 		);

	// 		T_ASSERT(!renderContext->havePendingDraws());

	// 		for (auto gathered : m_gathered)
	// 			gathered.entityRenderer->build(wc, worldRenderView, reflectionsPass, gathered.renderable);

	// 		for (auto entityRenderer : m_entityRenderers->get())
	// 			entityRenderer->build(wc, worldRenderView, reflectionsPass);
	// 	}
	// );

	 // Render screen space reflections.
	render::ImageGraphView view;
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.lastView = worldRenderView.getLastView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = worldRenderView.getDeltaTime();

	render::ImageGraphContext igctx;
	igctx.associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	igctx.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	igctx.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	igctx.associateTextureTargetSet(s_handleInputRoughness, gbufferTargetSetId, 0);

	m_screenReflections->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		igctx,
		view
	);

	renderGraph.addPass(rp);
	return reflectionsTargetSetId;
}

}
