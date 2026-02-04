/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/RTReflectionsPass.h"

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
#include "World/Entity/ProbeComponent.h"
#include "World/IrradianceGrid.h"
#include "World/IWorldRenderer.h"
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

const resource::Id< render::Shader > c_reflectionsComputeShader(L"{64878605-942D-6247-BA36-3D035727E2AE}");
const resource::Id< render::ImageGraph > c_reflectionsDenoise(L"{22777807-DD5F-1D47-8266-84EAA1EF024F}");

const render::Handle s_handleReflectionsOutput(L"World_ReflectionsOutput");

const render::Handle s_persistentReservoirBuffers[] = {
	render::Handle(L"World_RTReflections_Reservoir_Even"),
	render::Handle(L"World_RTReflections_Reservoir_Odd")
};

static Random s_random;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.RTReflectionsPass", RTReflectionsPass, Object)

bool RTReflectionsPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_reflectionsComputeShader, m_reflectionsComputeShader))
	{
		log::error << L"Unable to create reflections compute shader." << Endl;
		return false;
	}
	if (!resourceManager->bind(c_reflectionsDenoise, m_reflectionsDenoise))
	{
		log::error << L"Unable to create reflections denoiser process." << Endl;
		return false;
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	m_halfResolution = (bool)(desc.quality.reflections <= Quality::High);
	return true;
}

render::RGTargetSet RTReflectionsPass::setup(
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
	T_PROFILER_SCOPE(L"RTReflectionsPass::setup");
	T_FATAL_ASSERT(gatheredView.rtWorldTopLevel != nullptr);

	if (m_reflectionsComputeShader == nullptr || m_reflectionsDenoise == nullptr /* || gbufferTargetSetId == 0*/)
		return render::RGTargetSet::Invalid;

	// Add reservoir buffers.
	const render::RenderGraphBufferDesc reservoirBufferDesc = {
		.elementSize = sizeof(World_Reservoir_Type),
		.elementCount = 0,
		.referenceWidthDenom = m_halfResolution ? 2 : 1,
		.referenceHeightDenom = m_halfResolution ? 2 : 1
	};
	const DoubleBufferedBuffer reservoirBufferId = {
		renderGraph.addPersistentBuffer(L"RTReflections_Reservoir", s_persistentReservoirBuffers[frameCount % 2], reservoirBufferDesc),
		renderGraph.addPersistentBuffer(L"RTReflections_Reservoir", s_persistentReservoirBuffers[(frameCount + 1) % 2], reservoirBufferDesc)
	};

	// Add compute output reflections texture.
	const render::RenderGraphTextureDesc reflectionsTextureDesc = {
		.referenceWidthDenom = m_halfResolution ? 2 : 1,
		.referenceHeightDenom = m_halfResolution ? 2 : 1,
		.mipCount = 1,
		.format = render::TfR11G11B10F // Reflections (RGB)
	};
	const auto reflectionsTextureId = renderGraph.addTransientTexture(L"RTReflections", reflectionsTextureDesc);

	// Add final, up-sampled and denoised, reflections target.
	const render::RenderGraphTargetSetDesc reflectionsFinalTargetDesc = {
		.count = 1,
		.referenceWidthDenom = 1,
		.referenceHeightDenom = 1,
		.createDepthStencil = false,
		.targets = { {
			.colorFormat = render::TfR11G11B10F // Reflections (RGB)
		} }
	};
	const auto reflectionsFinalTargetSetId = renderGraph.addTransientTargetSet(L"Reflections", reflectionsFinalTargetDesc, render::RGTargetSet::Invalid, outputTargetSetId);

	// Shared shader parameters for all passes.
	const Vector2 jrc = needJitter ? jitter(frameCount) / worldRenderView.getViewSize() : Vector2::zero();
	const Vector2 jrp = needJitter ? jitter(frameCount - 1) / worldRenderView.getViewSize() : Vector2::zero();

	const ProbeComponent* probe = nullptr;
	for (auto p : gatheredView.probes)
	{
		if (!p->getLocal() && p->getTexture() != nullptr)
		{
			probe = p;
			break;
		}
	}

	auto setParameters = [=, this](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		const auto halfResDepthTexture = renderGraph.getTexture(halfResDepthTextureId);

		params->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		params->setVectorParameter(ShaderParameter::Jitter, Vector4(jrp.x, -jrp.y, jrc.x, -jrc.y)); // Texture space.
		params->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		params->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		params->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(ShaderParameter::GBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(ShaderParameter::GBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(ShaderParameter::GBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(ShaderParameter::HalfResDepthMap, m_halfResolution ? halfResDepthTexture : gbufferTargetSet->getColorTexture(0));
		params->setFloatParameter(ShaderParameter::Random, s_random.nextFloat());

		if (probe != nullptr)
		{
			params->setFloatParameter(ShaderParameter::ProbeIntensity, probe->getIntensity());
			params->setFloatParameter(ShaderParameter::ProbeTextureMips, (float)probe->getTexture()->getSize().mips);
			params->setTextureParameter(ShaderParameter::ProbeTexture, probe->getTexture());
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

	// Add reflections compute pass.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"RTReflections compute");
		rp->addInput(halfResDepthTextureId);
		rp->addInput(gbufferTargetSetId);
		rp->addInput(velocityTargetSetId);
		rp->addInput(reservoirBufferId.previous);
		rp->addInput(reservoirBufferId.current);
		rp->setOutput(reflectionsTextureId);
		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			render::ITexture* velocityTexture = renderGraph.getTargetSet(velocityTargetSetId)->getColorTexture(0);
			render::ITexture* reflectionsTexture = renderGraph.getTexture(reflectionsTextureId);
			render::Buffer* reservoirBuffer = renderGraph.getBuffer(reservoirBufferId.previous);
			render::Buffer* reservoirOutputBuffer = renderGraph.getBuffer(reservoirBufferId.current);

			const render::ITexture::Size outputSize = reflectionsTexture->getSize();

			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"RTReflections compute");
			renderBlock->program = m_reflectionsComputeShader->getProgram().program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->workSize[0] = outputSize.x;
			renderBlock->workSize[1] = outputSize.y;
			renderBlock->workSize[2] = 1;
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setTextureParameter(ShaderParameter::VelocityMap, velocityTexture);
			renderBlock->programParams->setBufferViewParameter(ShaderParameter::Reservoir, reservoirBuffer->getBufferView());
			renderBlock->programParams->setBufferViewParameter(ShaderParameter::ReservoirOutput, reservoirOutputBuffer->getBufferView());
			renderBlock->programParams->setImageViewParameter(s_handleReflectionsOutput, reflectionsTexture, 0);
			setParameters(renderGraph, renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			// renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, reflectionsTexture, 0);
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
		igctx.associateTexture(ShaderParameter::InputColor, reflectionsTextureId);
		igctx.associateTextureTargetSet(ShaderParameter::InputVelocity, velocityTargetSetId, 0);

		Ref< render::RenderPass > rp = new render::RenderPass(L"RTReflections denoiser");
		rp->addInput(gbufferTargetSetId);
		rp->addInput(velocityTargetSetId);
		rp->addInput(halfResDepthTextureId);
		rp->addInput(reflectionsTextureId);

		render::Clear clear;
		clear.mask = render::CfColor;
		clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
		rp->setOutput(reflectionsFinalTargetSetId, clear, render::TfNone, render::TfColor);

		m_reflectionsDenoise->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			igctx,
			view,
			setParameters);

		renderGraph.addPass(rp);
	}

	return reflectionsFinalTargetSetId;
}

}
