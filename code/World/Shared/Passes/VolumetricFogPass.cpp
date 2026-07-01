/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/VolumetricFogPass.h"

#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/FogComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_injectShader(Guid(L"{FEDA90CE-25C6-BC4D-9767-EA4B45F4A043}"));
const int32_t c_sliceCount = 128;

Random s_random;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VolumetricFogPass", VolumetricFogPass, Object)

VolumetricFogPass::VolumetricFogPass(const WorldRenderSettings& settings)
	: m_settings(settings)
{
}

bool VolumetricFogPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!resourceManager->bind(c_injectShader, m_injectShader))
		return false;

	const render::VolumeTextureCreateDesc vtcd = {
		.width = 128,
		.height = 128,
		.depth = c_sliceCount,
		.mipCount = 1,
		.format = render::TfR16G16B16A16F,
		.sRGB = false,
		.immutable = false,
		.shaderStorage = true
	};
	if ((m_volumeTextures[0] = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
		return false;
	if ((m_volumeTextures[1] = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
		return false;

	m_shadowsQuality = desc.quality.shadows;
	return true;
}

void VolumetricFogPass::destroy()
{
	safeDestroy(m_volumeTextures[0]);
	safeDestroy(m_volumeTextures[1]);
	m_injectShader.clear();
}

render::RGTexture VolumetricFogPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	const render::Buffer* lightSBuffer,
	const render::Buffer* tileSBuffer,
	const render::Buffer* lightIndexSBuffer,
	render::ITexture* whiteTexture,
	uint32_t frameCount,
	const float* slicePositions,
	render::RenderGraph& renderGraph,
	render::RGTargetSet shadowMapAtlasTargetSetId) const
{
	T_PROFILER_SCOPE(L"VolumetricFogPass::setup");

	Ref< const FogComponent > fog = gatheredView.fog;
	if (!fog || !fog->m_volumetricFogEnable)
		return render::RGTexture::Invalid;

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];

	const auto fogVolumeInputTextureId = renderGraph.addExplicitTexture(L"Fog volume input", m_volumeTextures[frameCount & 1]);
	const auto fogVolumeOutputTextureId = renderGraph.addExplicitTexture(L"Fog volume output", m_volumeTextures[1 - (frameCount & 1)]);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Volumetric fog");
	rp->setOutput(fogVolumeOutputTextureId);
	rp->addInput(fogVolumeInputTextureId);
	rp->addInput(shadowMapAtlasTargetSetId);

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);
		const auto fogVolumeInputTexture = renderGraph.getTexture(fogVolumeInputTextureId);
		const auto fogVolumeOutputTexture = renderGraph.getTexture(fogVolumeOutputTextureId);

		render::Shader::Permutation perm;
		m_injectShader->setCombination(ShaderPermutation::RayTracingEnable, (bool)(gatheredView.rtWorldTopLevel != nullptr), perm);
		const auto injectLightsProgram = m_injectShader->getProgram(perm);
		if (!injectLightsProgram)
			return;

		const auto& lastView = worldRenderView.getLastView();
		const auto& view = worldRenderView.getView();
		const auto& projection = worldRenderView.getProjection();

		const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
		const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();
		const float viewSliceScale = ClusterDimZ / std::log(viewFarZ / viewNearZ);
		const float viewSliceBias = ClusterDimZ * std::log(viewNearZ) / std::log(viewFarZ / viewNearZ) - 0.001f;

		const Frustum viewFrustum = worldRenderView.getViewFrustum();
		const Vector4 fogRange(
			viewFrustum.getNearZ(),
			std::min< float >(viewFrustum.getFarZ(), fog->m_maxDistance),
			0.0f,
			0.0f);

		const Scalar p11 = worldRenderView.getProjection().get(0, 0);
		const Scalar p22 = worldRenderView.getProjection().get(1, 1);

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Volumetric fog, inject analytical lights");

		renderBlock->program = injectLightsProgram.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->workSize[0] = 128;
		renderBlock->workSize[1] = 128;
		renderBlock->workSize[2] = c_sliceCount;

		renderBlock->programParams->beginParameters(renderContext);

		renderBlock->programParams->setFloatParameter(ShaderParameter::Time, worldRenderView.getTime());
		renderBlock->programParams->setFloatParameter(ShaderParameter::Random, s_random.nextFloat());
		renderBlock->programParams->setVectorParameter(ShaderParameter::ViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
		renderBlock->programParams->setVectorParameter(ShaderParameter::SlicePositions, Vector4(slicePositions[1], slicePositions[2], slicePositions[3], slicePositions[4]));
		renderBlock->programParams->setMatrixParameter(ShaderParameter::LastView, lastView);
		renderBlock->programParams->setMatrixParameter(ShaderParameter::LastViewInverse, lastView.inverse());
		renderBlock->programParams->setMatrixParameter(ShaderParameter::View, view);
		renderBlock->programParams->setMatrixParameter(ShaderParameter::ViewInverse, view.inverse());
		renderBlock->programParams->setMatrixParameter(ShaderParameter::Projection, projection);

		renderBlock->programParams->setBufferViewParameter(ShaderParameter::TileSBuffer, tileSBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(ShaderParameter::LightIndexSBuffer, lightIndexSBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(ShaderParameter::LightSBuffer, lightSBuffer->getBufferView());

		if (shadowAtlasTargetSet != nullptr)
		{
			renderBlock->programParams->setFloatParameter(ShaderParameter::ShadowBias, shadowSettings.bias);
			renderBlock->programParams->setTextureParameter(ShaderParameter::ShadowMapAtlas, shadowAtlasTargetSet->getDepthTexture());
		}
		else
		{
			renderBlock->programParams->setFloatParameter(ShaderParameter::ShadowBias, 0.0f);
			renderBlock->programParams->setTextureParameter(ShaderParameter::ShadowMapAtlas, whiteTexture);
		}

		if (gatheredView.rtWorldTopLevel != nullptr)
			renderBlock->programParams->setAccelerationStructureParameter(ShaderParameter::TLAS, gatheredView.rtWorldTopLevel);

		renderBlock->programParams->setTextureParameter(ShaderParameter::FogVolumeTexture, fogVolumeInputTexture);
		renderBlock->programParams->setImageViewParameter(ShaderParameter::FogVolume, fogVolumeOutputTexture, 0);

		renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeRange, fogRange);
		renderBlock->programParams->setVectorParameter(ShaderParameter::MagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeMediumColor, fog->m_mediumColor);
		renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeMediumDensity, fog->m_mediumDensity / c_sliceCount);
		renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeSliceCount, (float)c_sliceCount);
		renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeSliceUpdate, (float)(frameCount % c_sliceCount));

		renderBlock->programParams->endParameters(renderContext);

		renderContext->compute(renderBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, fogVolumeOutputTexture, 0);
	});

	renderGraph.addPass(rp);

	return fogVolumeOutputTextureId;
}

void VolumetricFogPass::setupSharedParameters(const GatherView& gatheredView, float viewNearZ, float viewFarZ, render::ProgramParameters* parameters)
{
	if (gatheredView.fog)
	{
		const Vector4 fogRange(
			viewNearZ,
			std::min< float >(viewFarZ, gatheredView.fog->m_maxDistance),
			gatheredView.fog->m_maxScattering,
			0.0f);

		// Distance fog.
		if (gatheredView.fog->m_distanceFogEnable)
		{
			parameters->setVectorParameter(ShaderParameter::FogDistanceAndDensity, Vector4(gatheredView.fog->m_fogDistance, gatheredView.fog->m_mediumDensity, 1.0f, gatheredView.fog->m_fogElevation));
			parameters->setVectorParameter(ShaderParameter::FogColor, gatheredView.fog->m_mediumColor);
		}
		else
		{
			parameters->setVectorParameter(ShaderParameter::FogDistanceAndDensity, Vector4::zero());
			parameters->setVectorParameter(ShaderParameter::FogColor, Vector4::zero());
		}

		// Volumetric fog.
		parameters->setFloatParameter(ShaderParameter::FogVolumeSliceCount, (float)c_sliceCount);
		parameters->setVectorParameter(ShaderParameter::FogVolumeRange, fogRange);
	}
	else
	{
		parameters->setVectorParameter(ShaderParameter::FogDistanceAndDensity, Vector4::zero());
		parameters->setVectorParameter(ShaderParameter::FogColor, Vector4::zero());
	}
}

}
