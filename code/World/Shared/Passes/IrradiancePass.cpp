/*
 * TRAKTOR
 * Copyright (c) 2024-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/IrradiancePass.h"

#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/LightComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
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

const resource::Id< render::ImageGraph > c_irradiance(L"{14A0E977-7C13-9B43-A26E-F1D21117AEC6}");

struct Reservoir
{
	uint32_t lightIndex;
	float W_light;
	float W_sum;
	float M;
};

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
	const render::Buffer* lightSBuffer,
	bool needJitter,
	uint32_t frameCount,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t velocityTargetSetId,
	render::handle_t halfResDepthTextureId,
	render::handle_t outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"IrradiancePass::setup");
	render::ImageGraphView view;

	if (m_irradiance == nullptr || gbufferTargetSetId == 0)
		return 0;

	const bool irradianceEnable = (bool)(gatheredView.irradianceGrid != nullptr);
	const bool irradianceSingle = (bool)(gatheredView.irradianceGrid != nullptr && gatheredView.irradianceGrid->isSingle());
	const bool rayTracingEnable = (bool)(gatheredView.rtWorldTopLevel != nullptr);

	// Add irradiance target set.
	const render::RenderGraphTargetSetDesc irradianceTargetDesc = {
		.count = 1,
		.referenceWidthDenom = 1,
		.referenceHeightDenom = 1,
		.createDepthStencil = false,
		.targets = { {
			.colorFormat = render::TfR11G11B10F // Irradiance (RGB)
		} }
	};
	const auto irradianceTargetSetId = renderGraph.addTransientTargetSet(L"Irradiance", irradianceTargetDesc, ~0U, outputTargetSetId);

	// Add reservoir buffers.
	const render::RenderGraphBufferDesc reservoirBufferDesc = {
		.elementSize = sizeof(Reservoir),
		.elementCount = 0,
		.referenceWidthMul = 1,
		.referenceWidthDenom = 1,
		.referenceHeightMul = 1,
		.referenceHeightDenom = 1
	};
	const auto reservoirBufferId = renderGraph.addPersistentBuffer(L"Reservoir", render::getParameterHandle(L"World_Reservoir_0"), reservoirBufferDesc);

	// Add ambient occlusion render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	render::ImageGraphContext igctx;
	igctx.setTechniqueFlag(s_handleIrradianceEnable, irradianceEnable);
	igctx.setTechniqueFlag(s_handleIrradianceSingle, irradianceSingle);
	igctx.setTechniqueFlag(s_handleRayTracingEnable, rayTracingEnable);
	igctx.associateTextureTargetSet(s_handleInputVelocity, velocityTargetSetId, 0);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance");
	rp->addInput(gbufferTargetSetId);
	rp->addInput(velocityTargetSetId);
	rp->addInput(halfResDepthTextureId);
	rp->addInput(reservoirBufferId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(irradianceTargetSetId, clear, render::TfNone, render::TfColor);

	const Vector2 jrc = needJitter ? jitter(frameCount) / worldRenderView.getViewSize() : Vector2::zero();
	const Vector2 jrp = needJitter ? jitter(frameCount - 1) / worldRenderView.getViewSize() : Vector2::zero();

	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		const auto halfResDepthTexture = renderGraph.getTexture(halfResDepthTextureId);
		const auto reservoirBuffer = renderGraph.getBuffer(reservoirBufferId);

		params->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
		params->setVectorParameter(s_handleJitter, Vector4(jrp.x, -jrp.y, jrc.x, -jrc.y)); // Texture space.
		params->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
		params->setMatrixParameter(s_handleView, worldRenderView.getView());
		params->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(s_handleGBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(s_handleGBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(s_handleGBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(s_handleHalfResDepthMap, halfResDepthTexture);

		if (gatheredView.irradianceGrid)
		{
			const auto size = gatheredView.irradianceGrid->getSize();
			params->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
			params->setVectorParameter(s_handleIrradianceGridBoundsMin, gatheredView.irradianceGrid->getBoundingBox().mn);
			params->setVectorParameter(s_handleIrradianceGridBoundsMax, gatheredView.irradianceGrid->getBoundingBox().mx);
			params->setBufferViewParameter(s_handleIrradianceGridSBuffer, gatheredView.irradianceGrid->getBuffer()->getBufferView());
		}

		if (lightSBuffer != nullptr)
		{
			params->setBufferViewParameter(s_handleLightSBuffer, lightSBuffer->getBufferView());
			params->setFloatParameter(s_handleLightCount, (float)gatheredView.lights.size());
		}
		else
			params->setFloatParameter(s_handleLightCount, 0.0f);

		if (reservoirBuffer != nullptr)
			params->setBufferViewParameter(s_handleReservoir, reservoirBuffer->getBufferView());

		if (gatheredView.rtWorldTopLevel != nullptr)
			params->setAccelerationStructureParameter(s_handleTLAS, gatheredView.rtWorldTopLevel);
	};

	m_irradiance->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		igctx,
		view,
		setParameters);

	renderGraph.addPass(rp);
	return irradianceTargetSetId;
}

}
