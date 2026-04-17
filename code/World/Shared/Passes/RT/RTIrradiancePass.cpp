/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/RT/RTIrradiancePass.h"

#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderSystem.h"
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
	float normal[3];
	float W_sum;
	float radiance[3];
	float M;
	float depth;
	uint8_t __pad__[12]; // 36
};

#pragma pack()

const resource::Id< render::Shader > c_irradianceFieldComputeShader(L"{593A522F-206C-9046-B8A1-25488BEA362A}");
const resource::Id< render::Shader > c_irradianceComputeShader(L"{7C871925-C1A9-5B47-A361-114BC8FB5A98}");
const resource::Id< render::ImageGraph > c_irradianceDenoise(L"{14A0E977-7C13-9B43-A26E-F1D21117AEC6}");

const render::Handle s_handleTechniqueIrradiance_RT(L"World_ComputeIrradiance_RT");
const render::Handle s_handleIrradianceOutput(L"World_IrradianceOutput");
const render::Handle s_handleIrradianceFieldImage(L"World_IrradianceFieldImage");
const render::Handle s_handleIrradianceFieldTexture(L"World_IrradianceFieldTexture");

static Random s_random;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.RTIrradiancePass", RTIrradiancePass, Object)

bool RTIrradiancePass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_irradianceFieldComputeShader, m_irradianceFieldComputeShader))
	{
		log::error << L"Unable to create irradiance grid compute shader." << Endl;
		return false;
	}
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

	// Create irradiance volume texture.
	render::VolumeTextureCreateDesc vtcd;
	vtcd.width = 128;
	vtcd.height = 128;
	vtcd.depth = 128;
	vtcd.mipCount = 1;
	vtcd.format = render::TfR16G16B16A16F;
	vtcd.sRGB = false;
	vtcd.immutable = false;
	vtcd.shaderStorage = true;
	if ((m_irradianceFieldTextures[0] = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
		return false;
	if ((m_irradianceFieldTextures[1] = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
		return false;

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	const std::wstring id = Guid::create().format();
	m_persistentReservoirBuffers[0] = render::Handle(render::getParameterHandle(std::wstring(L"World_Reservoir_Even_") + id));
	m_persistentReservoirBuffers[1] = render::Handle(render::getParameterHandle(std::wstring(L"World_Reservoir_Odd_") + id));

	// Use half-resolution output if high quality or less.
	m_halfResolution = (bool)(desc.quality.irradiance <= Quality::High);
	return true;
}

void RTIrradiancePass::destroy()
{
	safeDestroy(m_screenRenderer);
	m_irradianceComputeShader.clear();
	m_irradianceDenoise.clear();
}

render::RGTargetSet RTIrradiancePass::setup(
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
	T_PROFILER_SCOPE(L"RTIrradiancePass::setup");

	if (m_irradianceComputeShader == nullptr || m_irradianceDenoise == nullptr /* || gbufferTargetSetId == 0*/)
		return render::RGTargetSet::Invalid;

	const bool irradianceEnable = (bool)(gatheredView.irradianceGrid != nullptr);
	const bool irradianceSingle = (bool)(gatheredView.irradianceGrid != nullptr && gatheredView.irradianceGrid->isSingle());

	// Add reservoir buffers.
	const render::RenderGraphBufferDesc reservoirBufferDesc = {
		.elementSize = sizeof(World_Reservoir_Type),
		.elementCount = 0,
		.referenceWidthDenom = m_halfResolution ? 2 : 1,
		.referenceHeightDenom = m_halfResolution ? 2 : 1
	};
	const DoubleBufferedBuffer reservoirBufferId = {
		renderGraph.addPersistentBuffer(L"Reservoir", m_persistentReservoirBuffers[frameCount % 2], reservoirBufferDesc),
		renderGraph.addPersistentBuffer(L"Reservoir", m_persistentReservoirBuffers[(frameCount + 1) % 2], reservoirBufferDesc)
	};

	// Add compute output irradiance field volume texture.
	const auto irradianceFieldTexturePreviousId = renderGraph.addExplicitTexture(L"Irradiance field", m_irradianceFieldTextures[frameCount % 2]);
	const auto irradianceFieldTextureCurrentId = renderGraph.addExplicitTexture(L"Irradiance field", m_irradianceFieldTextures[(frameCount + 1) % 2]);

	// Add compute output irradiance texture.
	const render::RenderGraphTextureDesc irradianceTextureDesc = {
		.referenceWidthDenom = m_halfResolution ? 2 : 1,
		.referenceHeightDenom = m_halfResolution ? 2 : 1,
		.mipCount = 1,
		.format = render::TfR11G11B10F // Irradiance (RGB)
	};
	const auto irradianceTextureId = renderGraph.addTransientTexture(L"Irradiance", irradianceTextureDesc, gbufferTargetSetId);

	// Add final, up-sampled and denoised, irradiance target.
	const render::RenderGraphTargetSetDesc irradianceFinalTargetDesc = {
		.count = 1,
		.referenceWidthDenom = 1,
		.referenceHeightDenom = 1,
		.createDepthStencil = false,
		.targets = { {
			.colorFormat = render::TfR11G11B10F // Irradiance (RGB)
		} }
	};
	const auto irradianceFinalTargetSetId = renderGraph.addTransientTargetSet(L"Irradiance final", irradianceFinalTargetDesc, render::RGTargetSet::Invalid, outputTargetSetId);

	// Shared shader parameters for all passes.
	const Vector2 jrc = needJitter ? jitter(frameCount) / worldRenderView.getViewSize() : Vector2::zero();
	const Vector2 jrp = needJitter ? jitter(frameCount - 1) / worldRenderView.getViewSize() : Vector2::zero();
	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		const auto halfResDepthTexture = renderGraph.getTexture(halfResDepthTextureId);

		params->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		params->setVectorParameter(ShaderParameter::Jitter, Vector4(jrp.x, -jrp.y, jrc.x, -jrc.y)); // Texture space.
		params->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		params->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		params->setMatrixParameter(ShaderParameter::LastViewInverse, worldRenderView.getLastView().inverse());
		params->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(ShaderParameter::GBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(ShaderParameter::GBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(ShaderParameter::GBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(ShaderParameter::HalfResDepthMap, m_halfResolution ? halfResDepthTexture : gbufferTargetSet->getColorTexture(0));
		params->setFloatParameter(ShaderParameter::Random, s_random.nextFloat());

		if (gatheredView.irradianceGrid)
		{
			const auto size = gatheredView.irradianceGrid->getSize();
			params->setVectorParameter(ShaderParameter::IrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
			params->setVectorParameter(ShaderParameter::IrradianceGridBoundsMin, gatheredView.irradianceGrid->getBoundingBox().mn);
			params->setVectorParameter(ShaderParameter::IrradianceGridBoundsMax, gatheredView.irradianceGrid->getBoundingBox().mx);
			params->setBufferViewParameter(ShaderParameter::IrradianceGridSBuffer, gatheredView.irradianceGrid->getBuffer()->getBufferView());
		}

		if (lightSBuffer != nullptr)
		{
			params->setBufferViewParameter(ShaderParameter::LightSBuffer, lightSBuffer->getBufferView());
			params->setFloatParameter(ShaderParameter::LightCount, (float)gatheredView.lights.size());
		}
		else
			params->setFloatParameter(ShaderParameter::LightCount, 0.0f);

		params->setAccelerationStructureParameter(ShaderParameter::TLAS, gatheredView.rtWorldTopLevel);
	};

	// Add irradiance field compute pass.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance field compute");
		rp->addInput(irradianceFieldTexturePreviousId);
		rp->setOutput(irradianceFieldTextureCurrentId);
		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {

			render::ITexture* irradianceFieldTexturePrevious = renderGraph.getTexture(irradianceFieldTexturePreviousId);
			render::ITexture* irradianceFieldTextureCurrent = renderGraph.getTexture(irradianceFieldTextureCurrentId);

			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Irradiance field compute");
			renderBlock->program = m_irradianceFieldComputeShader->getProgram().program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->workSize[0] = 128;
			renderBlock->workSize[1] = 128;
			renderBlock->workSize[2] = 128;
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setTextureParameter(s_handleIrradianceFieldTexture, irradianceFieldTexturePrevious);
			renderBlock->programParams->setImageViewParameter(s_handleIrradianceFieldImage, irradianceFieldTextureCurrent, 0);
			setParameters(renderGraph, renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
		});

		renderGraph.addPass(rp);
	}

	// Add irradiance compute pass.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance compute");
		rp->addInput(halfResDepthTextureId);
		rp->addInput(gbufferTargetSetId);
		rp->addInput(velocityTargetSetId);
		rp->addInput(reservoirBufferId.previous);
		rp->addInput(reservoirBufferId.current);
		rp->addInput(irradianceFieldTextureCurrentId);
		rp->setOutput(irradianceTextureId);
		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			render::ITexture* velocityTexture = renderGraph.getTargetSet(velocityTargetSetId)->getColorTexture(0);
			render::ITexture* irradianceTexture = renderGraph.getTexture(irradianceTextureId);
			render::ITexture* irradianceFieldTexture = renderGraph.getTexture(irradianceFieldTextureCurrentId);
			render::Buffer* reservoirBuffer = renderGraph.getBuffer(reservoirBufferId.previous);
			render::Buffer* reservoirOutputBuffer = renderGraph.getBuffer(reservoirBufferId.current);

			const render::ITexture::Size outputSize = irradianceTexture->getSize();

			render::Shader::Permutation perm(s_handleTechniqueIrradiance_RT);
			m_irradianceComputeShader->setCombination(ShaderPermutation::IrradianceEnable, irradianceEnable, perm);
			m_irradianceComputeShader->setCombination(ShaderPermutation::IrradianceSingle, irradianceSingle, perm);

			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Irradiance compute");
			renderBlock->program = m_irradianceComputeShader->getProgram(perm).program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->workSize[0] = outputSize.x;
			renderBlock->workSize[1] = outputSize.y;
			renderBlock->workSize[2] = 1;
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setTextureParameter(ShaderParameter::VelocityMap, velocityTexture);
			renderBlock->programParams->setTextureParameter(s_handleIrradianceFieldTexture, irradianceFieldTexture);
			renderBlock->programParams->setBufferViewParameter(ShaderParameter::Reservoir, reservoirBuffer->getBufferView());
			renderBlock->programParams->setBufferViewParameter(ShaderParameter::ReservoirOutput, reservoirOutputBuffer->getBufferView());
			renderBlock->programParams->setImageViewParameter(s_handleIrradianceOutput, irradianceTexture, 0);
			setParameters(renderGraph, renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
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
		igctx.setTechniqueFlag(ShaderPermutation::IrradianceEnable, irradianceEnable);
		igctx.setTechniqueFlag(ShaderPermutation::IrradianceSingle, irradianceSingle);
		igctx.setTechniqueFlag(ShaderPermutation::RayTracingEnable, true);
		igctx.associateTexture(ShaderParameter::InputColor, irradianceTextureId);
		igctx.associateTextureTargetSet(ShaderParameter::InputVelocity, velocityTargetSetId, 0);

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
