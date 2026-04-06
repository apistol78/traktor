/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/SS/SSIrradiancePass.h"

#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
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

const resource::Id< render::Shader > c_irradianceComputeShader(L"{7C871925-C1A9-5B47-A361-114BC8FB5A98}");

const render::Handle s_handleTechniqueIrradiance(L"World_ComputeIrradiance");
const render::Handle s_handleIrradianceOutput(L"World_IrradianceOutput");

static Random s_random;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SSIrradiancePass", SSIrradiancePass, Object)

bool SSIrradiancePass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_irradianceComputeShader, m_irradianceComputeShader))
	{
		log::error << L"Unable to create irradiance compute shader." << Endl;
		return false;
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

void SSIrradiancePass::destroy()
{
	safeDestroy(m_screenRenderer);
	m_irradianceComputeShader.clear();
}

render::RGTargetSet SSIrradiancePass::setup(
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
	T_PROFILER_SCOPE(L"SSIrradiancePass::setup");

	if (m_irradianceComputeShader == nullptr || gbufferTargetSetId == render::RGTargetSet::Invalid)
		return render::RGTargetSet::Invalid;

	const bool halfResolution = false;
	const bool irradianceEnable = (bool)(gatheredView.irradianceGrid != nullptr);
	const bool irradianceSingle = (bool)(gatheredView.irradianceGrid != nullptr && gatheredView.irradianceGrid->isSingle());

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
		params->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
		params->setTextureParameter(ShaderParameter::GBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(ShaderParameter::GBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(ShaderParameter::GBufferC, gbufferTargetSet->getColorTexture(2));
		params->setTextureParameter(ShaderParameter::HalfResDepthMap, halfResolution ? halfResDepthTexture : gbufferTargetSet->getColorTexture(0));
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
	};

	// Add irradiance compute pass.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Irradiance compute");
		rp->addInput(halfResDepthTextureId);
		rp->addInput(gbufferTargetSetId);
		rp->addInput(velocityTargetSetId);
		rp->setOutput(irradianceFinalTargetSetId);
		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			render::ITexture* velocityTexture = renderGraph.getTargetSet(velocityTargetSetId)->getColorTexture(0);
			render::ITexture* irradianceTexture = renderGraph.getTargetSet(irradianceFinalTargetSetId)->getColorTexture(0);

			const render::ITexture::Size outputSize = irradianceTexture->getSize();

			render::Shader::Permutation perm(s_handleTechniqueIrradiance);
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
			renderBlock->programParams->setImageViewParameter(s_handleIrradianceOutput, irradianceTexture, 0);
			setParameters(renderGraph, renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, irradianceTexture, 0);
		});
		renderGraph.addPass(rp);
	}

	return irradianceFinalTargetSetId;
}

}
