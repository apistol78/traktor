/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/IWorldRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/IrradiancePass.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::ImageGraph > c_irradiance(L"{14A0E977-7C13-9B43-A26E-F1D21117AEC6}");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradiancePass", IrradiancePass, Object)

bool IrradiancePass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_irradiance, m_irradiance))
	{
		log::error << L"Unable to create irradiance process." << Endl;
		return false;
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

render::handle_t IrradiancePass::setup(
	const WorldRenderView& worldRenderView,
    const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"IrradiancePass::setup");
	render::ImageGraphView view;

	if (m_irradiance == nullptr || gbufferTargetSetId == 0)
		return 0;

	const bool irradianceEnable = (bool)(gatheredView.irradianceGrid != nullptr);
	const bool irradianceSingle = irradianceEnable && gatheredView.irradianceGrid->isSingle();

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8G8B8A8;	// Irradiance (RGB)

	auto irradianceTargetSetId = renderGraph.addTransientTargetSet(L"Irradiance", rgtd, ~0U, outputTargetSetId);

	// Add ambient occlusion render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	render::ImageGraphContext igctx;
	igctx.setTechniqueFlag(s_handleIrradianceEnable, irradianceEnable);
	igctx.setTechniqueFlag(s_handleIrradianceSingle, irradianceSingle);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance");
	rp->addInput(gbufferTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(irradianceTargetSetId, clear, render::TfNone, render::TfColor);

	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params)
	{
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		
		params->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
		params->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
		params->setMatrixParameter(s_handleView, worldRenderView.getView());
		params->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(s_handleGBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(s_handleGBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(s_handleGBufferC, gbufferTargetSet->getColorTexture(2));

		if (gatheredView.irradianceGrid)
		{
			const auto size = gatheredView.irradianceGrid->getSize();
			params->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
			params->setVectorParameter(s_handleIrradianceGridBoundsMin, gatheredView.irradianceGrid->getBoundingBox().mn);
			params->setVectorParameter(s_handleIrradianceGridBoundsMax, gatheredView.irradianceGrid->getBoundingBox().mx);
			params->setBufferViewParameter(s_handleIrradianceGridSBuffer, gatheredView.irradianceGrid->getBuffer()->getBufferView());
		}

		if (gatheredView.rtWorldTopLevel != nullptr)
			params->setAccelerationStructureParameter(s_handleTLAS, gatheredView.rtWorldTopLevel);
	};

	m_irradiance->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		igctx,
		view,
		setParameters
	);

	renderGraph.addPass(rp);
	return irradianceTargetSetId;
}

}
