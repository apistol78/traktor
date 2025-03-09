/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/AmbientOcclusionPass.h"

#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
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

render::RGTargetSet AmbientOcclusionPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	bool needJitter,
	uint32_t frameCount,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::handle_t halfResDepthTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"AmbientOcclusionPass::setup");
	render::ImageGraphView view;

	if (m_ambientOcclusion == nullptr || gbufferTargetSetId == render::RGTargetSet::Invalid)
		return render::RGTargetSet::Invalid;

	const bool rayTracingEnable = (bool)(gatheredView.rtWorldTopLevel != nullptr);
	const Matrix44& projection = worldRenderView.getProjection();
	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);
	const Vector4 magicCoeffs(1.0f / p11, 1.0f / p22, 0.0f, 0.0f);

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8; // Ambient occlusion (R)
	auto ambientOcclusionTargetSetId = renderGraph.addTransientTargetSet(L"Ambient occlusion", rgtd, render::RGTargetSet::Invalid, outputTargetSetId);

	// Add ambient occlusion render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	render::ImageGraphContext igctx;
	igctx.setTechniqueFlag(s_handleRayTracingEnable, rayTracingEnable);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");
	rp->addInput(gbufferTargetSetId);
	rp->addInput(halfResDepthTextureId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(ambientOcclusionTargetSetId, clear, render::TfNone, render::TfColor);

	const Vector2 jrc = needJitter ? jitter(frameCount) / worldRenderView.getViewSize() : Vector2::zero();
	const Vector2 jrp = needJitter ? jitter(frameCount - 1) / worldRenderView.getViewSize() : Vector2::zero();

	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		const auto halfResDepthTexture = renderGraph.getTexture(halfResDepthTextureId);

		params->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
		params->setVectorParameter(s_handleJitter, Vector4(jrp.x, -jrp.y, jrc.x, -jrc.y)); // Texture space.
		params->setVectorParameter(s_handleMagicCoeffs, magicCoeffs);
		params->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
		params->setMatrixParameter(s_handleView, worldRenderView.getView());
		params->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(s_handleGBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(s_handleGBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(s_handleGBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(s_handleHalfResDepthMap, halfResDepthTexture);

		if (gatheredView.rtWorldTopLevel != nullptr)
			params->setAccelerationStructureParameter(s_handleTLAS, gatheredView.rtWorldTopLevel);
	};

	m_ambientOcclusion->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		igctx,
		view,
		setParameters);

	renderGraph.addPass(rp);
	return ambientOcclusionTargetSetId;
}

}
