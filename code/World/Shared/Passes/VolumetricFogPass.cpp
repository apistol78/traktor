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
#include "World/IrradianceGrid.h"
#include "World/IWorldRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_integrateShader(Guid(L"{3C960DFE-C220-460C-9CF9-0182B798D4CB}"));
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
	if (!resourceManager->bind(c_integrateShader, m_integrateShader))
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
	if ((m_integratedTexture = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
		return false;

	m_shadowsQuality = desc.quality.shadows;
	return true;
}

void VolumetricFogPass::destroy()
{
	safeDestroy(m_volumeTextures[0]);
	safeDestroy(m_volumeTextures[1]);
	safeDestroy(m_integratedTexture);
	m_integrateShader.clear();
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
	if (!fog || !fog->m_volumetricFogEnable || !fog->m_mediumShader)
		return render::RGTexture::Invalid;

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];

	const auto fogVolumeInputTextureId = renderGraph.addExplicitTexture(L"Fog volume input", m_volumeTextures[frameCount & 1]);
	const auto fogVolumeOutputTextureId = renderGraph.addExplicitTexture(L"Fog volume output", m_volumeTextures[1 - (frameCount & 1)]);
	const auto fogVolumeIntegratedTextureId = renderGraph.addExplicitTexture(L"Fog volume integrated", m_integratedTexture);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Volumetric fog");
	rp->setOutput(fogVolumeIntegratedTextureId);
	rp->addInput(fogVolumeInputTextureId);
	rp->addInput(shadowMapAtlasTargetSetId);
	rp->addInput(gatheredView.rtWorldDependency);

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);
		const auto fogVolumeInputTexture = renderGraph.getTexture(fogVolumeInputTextureId);
		const auto fogVolumeOutputTexture = renderGraph.getTexture(fogVolumeOutputTextureId);
		const auto fogVolumeIntegratedTexture = renderGraph.getTexture(fogVolumeIntegratedTextureId);

		// Ambient in-scattering comes from the irradiance grid, so a scene without
		// one compiles the term out rather than sampling an absent grid 2M times
		// per frame.
		render::Shader::Permutation perm;
		fog->m_mediumShader->setCombination(ShaderPermutation::RayTracingEnable, (bool)(gatheredView.rtWorldTopLevel != nullptr), perm);
		fog->m_mediumShader->setCombination(ShaderPermutation::IrradianceEnable, (bool)(gatheredView.irradianceGrid != nullptr), perm);
		const auto injectLightsProgram = fog->m_mediumShader->getProgram(perm);
		if (!injectLightsProgram)
			return;

		const auto integrateProgram = m_integrateShader->getProgram();
		if (!integrateProgram)
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

		// Same grid the surface lighting samples, so fog ambient matches surfaces.
		if (gatheredView.irradianceGrid)
		{
			const auto size = gatheredView.irradianceGrid->getSize();
			renderBlock->programParams->setVectorParameter(ShaderParameter::IrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
			renderBlock->programParams->setVectorParameter(ShaderParameter::IrradianceGridBoundsMin, gatheredView.irradianceGrid->getBoundingBox().mn);
			renderBlock->programParams->setVectorParameter(ShaderParameter::IrradianceGridBoundsMax, gatheredView.irradianceGrid->getBoundingBox().mx);
			renderBlock->programParams->setBufferViewParameter(ShaderParameter::IrradianceGridSBuffer, gatheredView.irradianceGrid->getBuffer()->getBufferView());
		}

		renderBlock->programParams->setTextureParameter(ShaderParameter::FogVolumeTexture, fogVolumeInputTexture);
		renderBlock->programParams->setImageViewParameter(ShaderParameter::FogVolume, fogVolumeOutputTexture, 0);

		renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeRange, fogRange);
		renderBlock->programParams->setVectorParameter(ShaderParameter::MagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeMediumColor, fog->m_mediumColor);
		// Extinction per world unit, straight from the authored density. The
		// integration pass scales it by each froxel's real thickness, so it must
		// not be pre-divided by the slice count here.
		renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeMediumDensity, fog->m_mediumDensity);
		// Top of the fog, in world space. Uploaded regardless of whether the
		// distance fog is enabled - the volume has its own reason to respect it -
		// unlike the shared FogDistanceAndDensity which zeroes when it is off.
		renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeElevation, fog->m_fogElevation);

		// Two lobe phase function; g is clamped short of one because the forward
		// denominator is (1 - g)^2 and the phase diverges there.
		renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumePhase, Vector4(
			clamp(fog->m_phaseForward, -0.95f, 0.95f),
			clamp(fog->m_phaseBackward, -0.95f, 0.95f),
			clamp(fog->m_phaseBlend, 0.0f, 1.0f),
			0.0f));

		renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeSliceCount, (float)c_sliceCount);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->compute(renderBlock);

		// The integration reads every froxel the injection just wrote.
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, fogVolumeOutputTexture, 0);

		// Walk each froxel column front to back, turning the per froxel source
		// term and extinction into accumulated in-scattering and transmittance.
		// One thread per column, so the dispatch is flat in z.
		auto integrateBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Volumetric fog, integrate");

		integrateBlock->program = integrateProgram.program;
		integrateBlock->workSize[0] = 128;
		integrateBlock->workSize[1] = 128;
		integrateBlock->workSize[2] = 1;

		integrateBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		integrateBlock->programParams->beginParameters(renderContext);
		integrateBlock->programParams->setTextureParameter(ShaderParameter::FogVolumeTexture, fogVolumeOutputTexture);
		integrateBlock->programParams->setImageViewParameter(ShaderParameter::FogVolume, fogVolumeIntegratedTexture, 0);
		integrateBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeRange, fogRange);
		integrateBlock->programParams->endParameters(renderContext);

		renderContext->compute(integrateBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, fogVolumeIntegratedTexture, 0);
	});

	renderGraph.addPass(rp);

	return fogVolumeIntegratedTextureId;
}

void VolumetricFogPass::setupSharedParameters(const GatherView& gatheredView, float viewNearZ, float viewFarZ, render::ProgramParameters* parameters)
{
	if (gatheredView.fog)
	{
		const Vector4 fogRange(
			viewNearZ,
			std::min< float >(viewFarZ, gatheredView.fog->m_maxDistance),
			0.0f,
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
