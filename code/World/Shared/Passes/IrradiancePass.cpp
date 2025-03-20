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
#include "Core/Math/Random.h"
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

#pragma pack(1)

struct World_Reservoir_Type
{
	float position[3];
	uint8_t __pad__normal[4]; // 12
	float normal[3];
	uint8_t __pad__radiance[4]; // 28
	float radiance[3];
	float W;
	float W_sum;
	float M;
	uint8_t __pad__[8]; // 56
};

#pragma pack()

const resource::Id< render::Shader > c_irradianceComputeShader(L"{7C871925-C1A9-5B47-A361-114BC8FB5A98}");
const resource::Id< render::ImageGraph > c_irradianceDenoise(L"{14A0E977-7C13-9B43-A26E-F1D21117AEC6}");

const render::Handle s_handleTechniqueIrradiance(L"World_ComputeIrradiance");
const render::Handle s_handleTechniqueIrradiance_RT(L"World_ComputeIrradiance_RT");
const render::Handle s_handleIrradianceOutput(L"World_IrradianceOutput");

const render::Handle s_persistentReservoirBuffers[] = {
	render::Handle(L"World_Reservoir_Even"),
	render::Handle(L"World_Reservoir_Odd")
};

static Random s_random;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradiancePass", IrradiancePass, Object)

bool IrradiancePass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_irradianceComputeShader, m_irradianceComputeShader))
	{
		log::error << L"Unable to create irradiance compute shader." << Endl;
		return false;
	}
	if (!resourceManager->bind(c_irradianceDenoise, m_irradianceDenoise))
	{
		log::error << L"Unable to create irradiance denoiser process." << Endl;
		return false;
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

render::RGTargetSet IrradiancePass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	const render::Buffer* lightSBuffer,
	bool needJitter,
	uint32_t frameCount,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet velocityTargetSetId,
	render::RGTexture halfResDepthTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"IrradiancePass::setup");

	if (m_irradianceComputeShader == nullptr || m_irradianceDenoise == nullptr /* || gbufferTargetSetId == 0*/)
		return render::RGTargetSet::Invalid;

	const bool irradianceEnable = (bool)(gatheredView.irradianceGrid != nullptr);
	const bool irradianceSingle = (bool)(gatheredView.irradianceGrid != nullptr && gatheredView.irradianceGrid->isSingle());
	const bool rayTracingEnable = (bool)(gatheredView.rtWorldTopLevel != nullptr);

	// Add reservoir buffers.
	const render::RenderGraphBufferDesc reservoirBufferDesc = {
		.elementSize = sizeof(World_Reservoir_Type),
		.elementCount = 0,
		.referenceWidthDenom = 2,
		.referenceHeightDenom = 2
	};
	const DoubleBufferedBuffer reservoirBufferId = {
		renderGraph.addPersistentBuffer(L"Reservoir", s_persistentReservoirBuffers[frameCount % 2], reservoirBufferDesc),
		renderGraph.addPersistentBuffer(L"Reservoir", s_persistentReservoirBuffers[(frameCount + 1) % 2], reservoirBufferDesc)
	};

	// Add compute output irradiance texture.
	const render::RenderGraphTextureDesc irradianceTextureDesc = {
		.referenceWidthDenom = 2,
		.referenceHeightDenom = 2,
		.mipCount = 1,
		.format = render::TfR16G16B16A16F // Irradiance (RGB)
	};
	const auto irradianceTextureId = renderGraph.addTransientTexture(L"Irradiance", irradianceTextureDesc);

	// Add final, upsampled and denoised, irradiance target.
	const render::RenderGraphTargetSetDesc irradianceFinalTargetDesc = {
		.count = 1,
		.referenceWidthDenom = 1,
		.referenceHeightDenom = 1,
		.createDepthStencil = false,
		.targets = { {
			.colorFormat = render::TfR16G16B16A16F // Irradiance (RGB)
		} }
	};
	const auto irradianceFinalTargetSetId = renderGraph.addTransientTargetSet(L"Irradiance final", irradianceFinalTargetDesc, render::RGTargetSet::Invalid, outputTargetSetId);

	// Shared shader parameters for all passes.
	const Vector2 jrc = needJitter ? jitter(frameCount) / worldRenderView.getViewSize() : Vector2::zero();
	const Vector2 jrp = needJitter ? jitter(frameCount - 1) / worldRenderView.getViewSize() : Vector2::zero();
	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		const auto halfResDepthTexture = renderGraph.getTexture(halfResDepthTextureId);

		params->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
		params->setVectorParameter(s_handleJitter, Vector4(jrp.x, -jrp.y, jrc.x, -jrc.y)); // Texture space.
		params->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
		params->setMatrixParameter(s_handleView, worldRenderView.getView());
		params->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(s_handleGBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(s_handleGBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(s_handleGBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(s_handleHalfResDepthMap, halfResDepthTexture);
		params->setFloatParameter(s_handleRandom, s_random.nextFloat());

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

		if (gatheredView.rtWorldTopLevel != nullptr)
			params->setAccelerationStructureParameter(s_handleTLAS, gatheredView.rtWorldTopLevel);
	};

	// Add irradiance compute pass.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance compute");
		rp->addInput(velocityTargetSetId);
		rp->addInput(halfResDepthTextureId);
		rp->addInput(reservoirBufferId.previous);
		rp->addInput(reservoirBufferId.current);
		rp->setOutput(irradianceTextureId);
		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			render::ITexture* velocityTexture = renderGraph.getTargetSet(velocityTargetSetId)->getColorTexture(0);
			render::ITexture* halfResDepthTexture = renderGraph.getTexture(halfResDepthTextureId);
			render::ITexture* irradianceTexture = renderGraph.getTexture(irradianceTextureId);
			render::Buffer* reservoirBuffer = renderGraph.getBuffer(reservoirBufferId.previous);
			render::Buffer* reservoirOutputBuffer = renderGraph.getBuffer(reservoirBufferId.current);

			const render::ITexture::Size outputSize = irradianceTexture->getSize();

			render::Shader::Permutation perm(
				rayTracingEnable ? s_handleTechniqueIrradiance_RT : s_handleTechniqueIrradiance);
			m_irradianceComputeShader->setCombination(s_handleIrradianceEnable, irradianceEnable, perm);
			m_irradianceComputeShader->setCombination(s_handleIrradianceSingle, irradianceSingle, perm);

			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Irradiance compute");
			renderBlock->program = m_irradianceComputeShader->getProgram(perm).program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->workSize[0] = outputSize.x;
			renderBlock->workSize[1] = outputSize.y;
			renderBlock->workSize[2] = 1;
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setVectorParameter(s_handleReservoirSize, Vector4(outputSize.x, outputSize.y, 0.0f, 0.0f));
			renderBlock->programParams->setTextureParameter(s_handleVelocityMap, velocityTexture);
			renderBlock->programParams->setBufferViewParameter(s_handleReservoir, reservoirBuffer->getBufferView());
			renderBlock->programParams->setBufferViewParameter(s_handleReservoirOutput, reservoirOutputBuffer->getBufferView());
			renderBlock->programParams->setImageViewParameter(s_handleIrradianceOutput, irradianceTexture, 0);
			setParameters(renderGraph, renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			// renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, irradianceTexture, 0);
		});
		renderGraph.addPass(rp);
	}

	// Add denoiser render pass.
	{
		render::ImageGraphView view;
		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getView();
		view.projection = worldRenderView.getProjection();

		render::ImageGraphContext igctx;
		igctx.setTechniqueFlag(s_handleIrradianceEnable, irradianceEnable);
		igctx.setTechniqueFlag(s_handleIrradianceSingle, irradianceSingle);
		igctx.setTechniqueFlag(s_handleRayTracingEnable, rayTracingEnable);
		igctx.associateTexture(s_handleInputColor, irradianceTextureId);
		igctx.associateTextureTargetSet(s_handleInputVelocity, velocityTargetSetId, 0);

		Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance denoiser");
		rp->addInput(gbufferTargetSetId);
		rp->addInput(velocityTargetSetId);
		rp->addInput(halfResDepthTextureId);
		rp->addInput(irradianceTextureId);

		render::Clear clear;
		clear.mask = render::CfColor;
		clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
		rp->setOutput(irradianceFinalTargetSetId, clear, render::TfNone, render::TfColor);

		m_irradianceDenoise->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			igctx,
			view,
			setParameters);

		renderGraph.addPass(rp);
	}

	return irradianceFinalTargetSetId;
}

}
