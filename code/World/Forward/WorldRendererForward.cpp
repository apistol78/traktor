/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Range.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/VolumetricFogComponent.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"

namespace traktor::world
{
	namespace
	{

const render::Handle s_handleVisualTargetSet[] =
{
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

Vector2 jitter(int32_t count)
{
	const Vector2 kernelSize(0.5f, 0.5f);
	return Vector2(
		(float)((count / 2) & 1) * kernelSize.x - kernelSize.x / 2.0f,
		(float)(count & 1) * kernelSize.y - kernelSize.y / 2.0f
	);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererForward", 0, WorldRendererForward, WorldRendererShared)

bool WorldRendererForward::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	if (!WorldRendererShared::create(
		resourceManager,
		renderSystem,
		desc
	))
		return false;

	return true;
}

void WorldRendererForward::setup(
	const WorldRenderView& immutableWorldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	WorldRenderView worldRenderView = immutableWorldRenderView;

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
	if (m_postProcessPass->needCameraJitter())
	{
		const Vector2 ndc = (jitter(m_count) * 2.0f) / worldRenderView.getViewSize();
		Matrix44 proj = immutableWorldRenderView.getProjection();
		proj = translate(ndc.x, ndc.y, 0.0f) * proj;
		worldRenderView.setProjection(proj);
	}

	// Gather active renderables for this frame.
	gather(const_cast< Entity* >(rootEntity));

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererForward setup extra passes");
		WorldSetupContext context(m_entityRenderers, m_irradianceGrid, rootEntity, renderGraph);

		for (auto r : m_gatheredView.renderables)
			r.renderer->setup(context, worldRenderView, r.renderable);
	
		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add visual target sets.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;
	auto visualReadTargetSetId = renderGraph.addPersistentTargetSet(L"History", s_handleVisualTargetSet[m_count % 2], false, rgtd, m_sharedDepthStencil, outputTargetSetId);
	auto visualWriteTargetSetId = renderGraph.addPersistentTargetSet(L"Visual", s_handleVisualTargetSet[(m_count + 1) % 2], false, rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add passes to render graph.
	m_lightClusterPass->setup(worldRenderView, m_gatheredView);
	auto gbufferTargetSetId = m_gbufferPass->setup(worldRenderView, rootEntity, m_gatheredView, nullptr, s_techniqueForwardGBufferWrite, renderGraph, outputTargetSetId);
	auto velocityTargetSetId = m_velocityPass->setup(worldRenderView, rootEntity, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto ambientOcclusionTargetSetId = m_ambientOcclusionPass->setup(worldRenderView, rootEntity, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto reflectionsTargetSetId = m_reflectionsPass->setup(worldRenderView, rootEntity, m_gatheredView, m_blackCubeTexture, renderGraph, gbufferTargetSetId, visualReadTargetSetId, outputTargetSetId);

	render::handle_t shadowMapAtlasTargetSetId = 0;
	setupLightPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		shadowMapAtlasTargetSetId
	);

	setupVisualPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		visualWriteTargetSetId,
		gbufferTargetSetId,
		ambientOcclusionTargetSetId,
		reflectionsTargetSetId,
		shadowMapAtlasTargetSetId
	);

	m_postProcessPass->setup(worldRenderView, rootEntity, m_gatheredView, m_count, renderGraph, gbufferTargetSetId, velocityTargetSetId, visualWriteTargetSetId, visualReadTargetSetId, outputTargetSetId);

	m_count++;
}

void WorldRendererForward::setupVisualPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t visualWriteTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t ambientOcclusionTargetSetId,
	render::handle_t reflectionsTargetSetId,
	render::handle_t shadowMapAtlasTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupVisualPass");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

	// Use first volumetric fog volume, only support one in forward.
	const VolumetricFogComponent* fog = (!worldRenderView.getSnapshot() && !m_gatheredView.fogs.empty()) ? m_gatheredView.fogs.front() : nullptr;

	// Create render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);

	if (ambientOcclusionTargetSetId != 0)
		rp->addInput(ambientOcclusionTargetSetId);
	if (reflectionsTargetSetId != 0)
		rp->addInput(reflectionsTargetSetId);
	if (shadowsEnable)
		rp->addInput(shadowMapAtlasTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(visualWriteTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			const auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
			const auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
			const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);

			const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
			const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();
			const float viewSliceScale = ClusterDimZ / std::log(viewFarZ / viewNearZ);
			const float viewSliceBias = ClusterDimZ * std::log(viewNearZ) / std::log(viewFarZ / viewNearZ) - 0.001f;

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(wc.getRenderContext());
			sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
			sharedParams->setVectorParameter(s_handleViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
			sharedParams->setVectorParameter(s_handleSlicePositions, Vector4(m_slicePositions[1], m_slicePositions[2], m_slicePositions[3], m_slicePositions[4]));
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());

			if (m_irradianceGrid)
			{
				const auto size = m_irradianceGrid->getSize();
				sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
				sharedParams->setBufferViewParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer()->getBufferView());
			}

			sharedParams->setBufferViewParameter(s_handleTileSBuffer, m_lightClusterPass->getTileSBuffer()->getBufferView());
			sharedParams->setBufferViewParameter(s_handleLightIndexSBuffer, m_lightClusterPass->getLightIndexSBuffer()->getBufferView());
			sharedParams->setBufferViewParameter(s_handleLightSBuffer, m_lightSBuffer->getBufferView());

			if (fog)
			{
				const Vector4 fogRange(
					viewNearZ,
					std::min< float >(viewFarZ, fog->getMaxDistance()),
					0.0f,
					0.0f
				);

				sharedParams->setFloatParameter(s_handleFogVolumeSliceCount, (float)fog->getSliceCount());
				sharedParams->setVectorParameter(s_handleFogVolumeRange, fogRange);
				sharedParams->setTextureParameter(s_handleFogVolumeTexture, fog->getFogVolumeTexture());
			}

			sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, Vector4(m_settings.fogDistance, m_settings.fogDensity, m_settings.fogDensityMax, 0.0f));
			sharedParams->setVectorParameter(s_handleFogColor, m_settings.fogColor);

			if (shadowAtlasTargetSet != nullptr)
			{
				sharedParams->setFloatParameter(s_handleShadowBias, shadowSettings.bias);
				sharedParams->setTextureParameter(s_handleShadowMapAtlas, shadowAtlasTargetSet->getDepthTexture());
			}
			else
			{
				sharedParams->setFloatParameter(s_handleShadowBias, 0.0f);
				sharedParams->setTextureParameter(s_handleShadowMapAtlas, m_whiteTexture);
			}

			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));

			if (ambientOcclusionTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleOcclusionMap, m_whiteTexture);

			if (reflectionsTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleReflectionMap, reflectionsTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleReflectionMap, m_blackTexture);

			sharedParams->endParameters(wc.getRenderContext());

			const WorldRenderPassShared defaultPass(
				s_techniqueForwardColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::Last,
				{
					{ s_handleIrradianceEnable, (bool)(m_irradianceGrid != nullptr) },
					{ s_handleVolumetricFogEnable, (bool)(fog != nullptr )}
				}
			);

			T_ASSERT(!wc.getRenderContext()->havePendingDraws());

			for (auto r : m_gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, defaultPass, r.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, defaultPass);
		}
	);

	renderGraph.addPass(rp);
}

}
