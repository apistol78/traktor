/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Forward/WorldRendererForward.h"

#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Range.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/Entity/FogComponent.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/DBufferPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/HiZPass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <cstring>

namespace traktor::world
{
namespace
{

const render::Handle s_handleVisualTargetSet[] = {
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererForward", 0, WorldRendererForward, WorldRendererShared)

bool WorldRendererForward::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc)
{
	if (!WorldRendererShared::create(
			resourceManager,
			renderSystem,
			desc))
		return false;

	return true;
}

void WorldRendererForward::setup(
	const World* world,
	const WorldRenderView& immutableWorldRenderView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet outputTargetSetId,
	const std::function< bool(const EntityState& state) >& filter)
{
	WorldRenderView worldRenderView = immutableWorldRenderView;
	const uint32_t count = m_state[worldRenderView.getIndex()].count;
	const bool needJitter = m_postProcessPass->needCameraJitter();

#if defined(T_WORLD_USE_TILE_JOB)
	// Ensure tile job is finished, this should never happen since it will indicate
	// previous frame hasn't been rendered.
	if (m_tileJob != nullptr)
	{
		m_tileJob->wait();
		m_tileJob = nullptr;
	}
#endif

	// Jitter projection for TAA, calculate jitter in clip space.
	if (needJitter)
	{
		const Vector2 ndc = (jitter(count) * 2.0f) / worldRenderView.getViewSize();
		Matrix44 proj = immutableWorldRenderView.getProjection();
		proj = translate(ndc.x, ndc.y, 0.0f) * proj;
		worldRenderView.setProjection(proj);
	}

	// Gather active renderables for this frame.
	gather(world, filter);

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererForward setup extra passes");
		WorldSetupContext context(world, m_entityRenderers, renderGraph, m_visualAttachments);

		for (const auto& r : m_gatheredView.renderables)
			r.renderer->setup(context, worldRenderView, r.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add visual target sets.
	const render::RenderGraphTargetSetDesc rgtd = {
		.count = 1,
		.referenceWidthDenom = 1,
		.referenceHeightDenom = 1,
		.createDepthStencil = false,
		.targets = { { .colorFormat = render::TfR11G11B10F } }
	};
	const DoubleBufferedTarget visualTargetSetId = {
		renderGraph.addPersistentTargetSet(L"Previous", s_handleVisualTargetSet[count % 2], false, rgtd, outputTargetSetId, outputTargetSetId),
		renderGraph.addPersistentTargetSet(L"Current", s_handleVisualTargetSet[(count + 1) % 2], false, rgtd, outputTargetSetId, outputTargetSetId)
	};

	const render::Buffer* lightSBuffer = m_state[worldRenderView.getIndex()].lightSBuffer;

	// Add passes to render graph.
	m_lightClusterPass->setup(worldRenderView, m_gatheredView);
	auto gbufferTargetSetId = m_gbufferPass->setup(worldRenderView, m_gatheredView, ShaderTechnique::ForwardGBufferWrite, renderGraph, render::RGTexture::Invalid, outputTargetSetId);
	auto dbufferTargetSetId = m_dbufferPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	// m_hiZPass->setup(worldRenderView, renderGraph, gbufferTargetSetId);
	auto velocityTargetSetId = m_velocityPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto ambientOcclusionTargetSetId = m_ambientOcclusionPass->setup(worldRenderView, m_gatheredView, needJitter, count, renderGraph, gbufferTargetSetId, render::RGTexture::Invalid, outputTargetSetId);
	auto reflectionsTargetSetId = m_reflectionsPass->setup(worldRenderView, m_gatheredView, lightSBuffer, m_blackCubeTexture, needJitter, count, renderGraph, gbufferTargetSetId, dbufferTargetSetId, visualTargetSetId.previous, velocityTargetSetId, render::RGTexture::Invalid, outputTargetSetId);

	render::RGTargetSet shadowMapAtlasTargetSetId;
	setupLightPass(
		worldRenderView,
		renderGraph,
		outputTargetSetId,
		shadowMapAtlasTargetSetId);

	setupVisualPass(
		worldRenderView,
		renderGraph,
		visualTargetSetId.current,
		visualTargetSetId.previous,
		gbufferTargetSetId,
		ambientOcclusionTargetSetId,
		reflectionsTargetSetId,
		shadowMapAtlasTargetSetId);

	m_postProcessPass->setup(worldRenderView, m_gatheredView, count, m_whiteTexture, renderGraph, gbufferTargetSetId, velocityTargetSetId, visualTargetSetId, outputTargetSetId);

	m_state[worldRenderView.getIndex()].count++;
}

void WorldRendererForward::setupVisualPass(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet visualWriteTargetSetId,
	render::RGTargetSet visualReadTargetSetId,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet ambientOcclusionTargetSetId,
	render::RGTargetSet reflectionsTargetSetId,
	render::RGTargetSet shadowMapAtlasTargetSetId)
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupVisualPass");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

	// Find first, non-local, probe for reflections on transparent surfaces.
	const ProbeComponent* probe = nullptr;
	for (auto p : m_gatheredView.probes)
	{
		if (!p->getLocal() && p->getTexture() != nullptr)
		{
			probe = p;
			break;
		}
	}

	// Get volumetric fog volume.
	const FogComponent* fog = m_gatheredView.fog;

	// Create render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);

	if (ambientOcclusionTargetSetId != render::RGTargetSet::Invalid)
		rp->addInput(ambientOcclusionTargetSetId);
	if (reflectionsTargetSetId != render::RGTargetSet::Invalid)
		rp->addInput(reflectionsTargetSetId);
	if (shadowsEnable)
		rp->addInput(shadowMapAtlasTargetSetId);

	for (auto attachment : m_visualAttachments)
		rp->addInput(attachment);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(visualWriteTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	Ref< render::Buffer > lightSBuffer = m_state[worldRenderView.getIndex()].lightSBuffer;

	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const WorldBuildContext wc(
			m_entityRenderers,
			renderContext);

		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		const auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
		const auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
		const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);
		const auto visualCopyTargetSet = renderGraph.getTargetSet(visualReadTargetSetId);

		const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
		const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();
		const float viewSliceScale = ClusterDimZ / std::log(viewFarZ / viewNearZ);
		const float viewSliceBias = ClusterDimZ * std::log(viewNearZ) / std::log(viewFarZ / viewNearZ) - 0.001f;

		auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(wc.getRenderContext());
		sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		sharedParams->setVectorParameter(ShaderParameter::ViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
		sharedParams->setVectorParameter(ShaderParameter::SlicePositions, Vector4(m_slicePositions[1], m_slicePositions[2], m_slicePositions[3], m_slicePositions[4]));
		sharedParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		sharedParams->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());

		if (m_gatheredView.irradianceGrid)
		{
			const auto size = m_gatheredView.irradianceGrid->getSize();
			sharedParams->setVectorParameter(ShaderParameter::IrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
			sharedParams->setVectorParameter(ShaderParameter::IrradianceGridBoundsMin, m_gatheredView.irradianceGrid->getBoundingBox().mn);
			sharedParams->setVectorParameter(ShaderParameter::IrradianceGridBoundsMax, m_gatheredView.irradianceGrid->getBoundingBox().mx);
			sharedParams->setBufferViewParameter(ShaderParameter::IrradianceGridSBuffer, m_gatheredView.irradianceGrid->getBuffer()->getBufferView());
		}

		sharedParams->setBufferViewParameter(ShaderParameter::TileSBuffer, m_lightClusterPass->getTileSBuffer()->getBufferView());
		sharedParams->setBufferViewParameter(ShaderParameter::LightIndexSBuffer, m_lightClusterPass->getLightIndexSBuffer()->getBufferView());
		sharedParams->setBufferViewParameter(ShaderParameter::LightSBuffer, lightSBuffer->getBufferView());

		if (probe)
		{
			sharedParams->setFloatParameter(ShaderParameter::ProbeIntensity, probe->getIntensity());
			sharedParams->setFloatParameter(ShaderParameter::ProbeTextureMips, (float)probe->getTexture()->getSize().mips);
			sharedParams->setTextureParameter(ShaderParameter::ProbeTexture, probe->getTexture());
		}
		else
		{
			sharedParams->setFloatParameter(ShaderParameter::ProbeIntensity, 0.0f);
			sharedParams->setFloatParameter(ShaderParameter::ProbeTextureMips, 0.0f);
			sharedParams->setTextureParameter(ShaderParameter::ProbeTexture, m_blackCubeTexture);
		}

		if (fog)
		{
			const Vector4 fogRange(
				viewNearZ,
				std::min< float >(viewFarZ, fog->getMaxDistance()),
				fog->getMaxScattering(),
				0.0f);

			// Distance fog.
			sharedParams->setVectorParameter(ShaderParameter::FogDistanceAndDensity, Vector4(fog->m_fogDistance, fog->m_fogDensity, fog->m_fogDensityMax, 0.0f));
			sharedParams->setVectorParameter(ShaderParameter::FogColor, fog->m_fogColor);

			// Volumetric fog.
			sharedParams->setFloatParameter(ShaderParameter::FogVolumeSliceCount, (float)fog->getSliceCount());
			sharedParams->setVectorParameter(ShaderParameter::FogVolumeRange, fogRange);
			sharedParams->setTextureParameter(ShaderParameter::FogVolumeTexture, fog->getFogVolumeTexture());
		}
		else
		{
			sharedParams->setVectorParameter(ShaderParameter::FogDistanceAndDensity, Vector4::zero());
			sharedParams->setVectorParameter(ShaderParameter::FogColor, Vector4::zero());
		}

		if (shadowAtlasTargetSet != nullptr)
		{
			sharedParams->setFloatParameter(ShaderParameter::ShadowBias, shadowSettings.bias);
			sharedParams->setTextureParameter(ShaderParameter::ShadowMapAtlas, shadowAtlasTargetSet->getDepthTexture());
		}
		else
		{
			sharedParams->setFloatParameter(ShaderParameter::ShadowBias, 0.0f);
			sharedParams->setTextureParameter(ShaderParameter::ShadowMapAtlas, m_whiteTexture);
		}

		sharedParams->setTextureParameter(ShaderParameter::GBufferA, gbufferTargetSet->getColorTexture(0));
		sharedParams->setTextureParameter(ShaderParameter::GBufferB, gbufferTargetSet->getColorTexture(1));
		sharedParams->setTextureParameter(ShaderParameter::GBufferC, gbufferTargetSet->getColorTexture(2));

		if (ambientOcclusionTargetSet != nullptr)
			sharedParams->setTextureParameter(ShaderParameter::OcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
		else
			sharedParams->setTextureParameter(ShaderParameter::OcclusionMap, m_whiteTexture);

		if (reflectionsTargetSet != nullptr)
			sharedParams->setTextureParameter(ShaderParameter::ReflectionMap, reflectionsTargetSet->getColorTexture(0));
		else
			sharedParams->setTextureParameter(ShaderParameter::ReflectionMap, m_blackTexture);

		if (visualCopyTargetSet != nullptr)
			sharedParams->setTextureParameter(ShaderParameter::VisualCopyMap, visualCopyTargetSet->getColorTexture(0));
		else
			sharedParams->setTextureParameter(ShaderParameter::VisualCopyMap, m_blackTexture);

		if (m_gatheredView.rtWorldTopLevel != nullptr)
			sharedParams->setAccelerationStructureParameter(ShaderParameter::TLAS, m_gatheredView.rtWorldTopLevel);

		sharedParams->endParameters(wc.getRenderContext());

		const bool irradianceEnable = (bool)(m_gatheredView.irradianceGrid != nullptr);
		const bool irradianceSingle = irradianceEnable && m_gatheredView.irradianceGrid->isSingle();

		const WorldRenderPassShared defaultPass(
			ShaderTechnique::ForwardColor,
			sharedParams,
			worldRenderView,
			IWorldRenderPass::Last,
			{ { ShaderPermutation::IrradianceEnable, irradianceEnable },
				{ ShaderPermutation::IrradianceSingle, irradianceSingle },
				{ ShaderPermutation::VolumetricFogEnable, (bool)(fog != nullptr && fog->m_volumetricFogEnable) } });

		T_ASSERT(!wc.getRenderContext()->havePendingDraws());

		for (const auto& r : m_gatheredView.renderables)
			r.renderer->build(wc, worldRenderView, defaultPass, r.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->build(wc, worldRenderView, defaultPass);
	});

	renderGraph.addPass(rp);
}

}
