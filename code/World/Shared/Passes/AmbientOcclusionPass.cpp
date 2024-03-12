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
#include "Render/IRenderTargetSet.h"
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
#include "World/Shared/Passes/AmbientOcclusionPass.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{416745F9-93C7-8D45-AE28-F2823DEE636A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{45F9CD9F-C700-9942-BB36-443629C88748}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{302E57C8-711D-094F-A764-75F76553E81B}");

resource::Id< render::ImageGraph > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case Quality::Disabled:
		return resource::Id< render::ImageGraph >();
	case Quality::Low:
		return c_ambientOcclusionLow;
	case Quality::Medium:
		return c_ambientOcclusionMedium;
	case Quality::High:
		return c_ambientOcclusionHigh;
	case Quality::Ultra:
		return c_ambientOcclusionUltra;
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.AmbientOcclusionPass", AmbientOcclusionPass, Object)

AmbientOcclusionPass::AmbientOcclusionPass(
    const WorldRenderSettings& settings,
    WorldEntityRenderers* entityRenderers
)
:   m_settings(settings)
,   m_entityRenderers(entityRenderers)
{
}

bool AmbientOcclusionPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	// Create ambient occlusion processing.
	if (desc.quality.ambientOcclusion > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > ambientOcclusion = getAmbientOcclusionId(desc.quality.ambientOcclusion);
		if (!resourceManager->bind(ambientOcclusion, m_ambientOcclusion))
		{
			log::error << L"Unable to create ambient occlusion process." << Endl;
			return false;
		}
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

render::handle_t AmbientOcclusionPass::setup(
	const WorldRenderView& worldRenderView,
    const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"AmbientOcclusionPass::setup");
	render::ImageGraphView view;

	if (m_ambientOcclusion == nullptr || gbufferTargetSetId == 0)
		return 0;

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	auto ambientOcclusionTargetSetId = renderGraph.addTransientTargetSet(L"Ambient occlusion", rgtd, ~0U, outputTargetSetId);

	// Add ambient occlusion render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	render::ImageGraphContext igctx;

	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");
	rp->addInput(gbufferTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(ambientOcclusionTargetSetId, clear, render::TfNone, render::TfColor);

	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		params->setTextureParameter(s_handleGBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(s_handleGBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(s_handleGBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(s_handleGBufferD, gbufferTargetSet->getColorTexture(3));
	};

	m_ambientOcclusion->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		igctx,
		view,
		setParameters
	);

	renderGraph.addPass(rp);
	return ambientOcclusionTargetSetId;
}

}
