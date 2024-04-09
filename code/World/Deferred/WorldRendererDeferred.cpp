/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
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
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/ContactShadowsPass.h"
#include "World/Shared/Passes/DBufferPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/HiZPass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"

namespace traktor::world
{
	namespace
	{

const render::Handle s_persistentVisualTargetSet[] =
{
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

const resource::Id< render::Shader > c_lightShader(L"{707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}");

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, WorldRendererShared)

bool WorldRendererDeferred::create(
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

	// Create light, reflection and fog shaders.
	if (!resourceManager->bind(c_lightShader, m_lightShader))
		return false;

	return true;
}

void WorldRendererDeferred::destroy()
{
	WorldRendererShared::destroy();
	m_lightShader.clear();
}

void WorldRendererDeferred::setup(
	const World* world,
	const WorldRenderView& immutableWorldRenderView,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	const std::function< bool(const EntityState& state) >& filter
)
{
	WorldRenderView worldRenderView = immutableWorldRenderView;
	const uint32_t count = m_state[worldRenderView.getIndex()].count;

	// Jitter projection for TAA, calculate jitter in clip space.
	if (m_postProcessPass->needCameraJitter())
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
		T_PROFILER_SCOPE(L"WorldRendererDeferred setup extra passes");
		WorldSetupContext context(world, m_entityRenderers, m_irradianceGrid, renderGraph, m_visualAttachments);

		for (const auto& r : m_gatheredView.renderables)
			r.renderer->setup(context, worldRenderView, r.renderable);
	
		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add visual target sets.
	render::RenderGraphTargetSetDesc rgtsd;
	rgtsd.count = 1;
	rgtsd.createDepthStencil = false;
	rgtsd.referenceWidthDenom = 1;
	rgtsd.referenceHeightDenom = 1;
	rgtsd.targets[0].colorFormat = render::TfR16G16B16A16F;
	const DoubleBufferedTarget visualTargetSetId =
	{
		renderGraph.addPersistentTargetSet(L"Previous", s_persistentVisualTargetSet[count % 2], false, rgtsd, outputTargetSetId, outputTargetSetId),
		renderGraph.addPersistentTargetSet(L"Current", s_persistentVisualTargetSet[(count + 1) % 2], false, rgtsd, outputTargetSetId, outputTargetSetId)
	};
	
	// Add Hi-Z texture.
	const render::handle_t hizTextureId = m_hiZPass->addTexture(worldRenderView, renderGraph);

	// Add passes to render graph.
	m_lightClusterPass->setup(worldRenderView, m_gatheredView);
	auto gbufferTargetSetId = m_gbufferPass->setup(worldRenderView, m_gatheredView, m_irradianceGrid, s_techniqueDeferredGBufferWrite, renderGraph, hizTextureId, outputTargetSetId);
	auto dbufferTargetSetId = m_dbufferPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	m_hiZPass->setup(worldRenderView, renderGraph, gbufferTargetSetId, hizTextureId);
	auto velocityTargetSetId = m_velocityPass->setup(worldRenderView, m_gatheredView, count, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto ambientOcclusionTargetSetId = m_ambientOcclusionPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto contactShadowsTargetSetId = m_contactShadowsPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto reflectionsTargetSetId = m_reflectionsPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, dbufferTargetSetId, visualTargetSetId.previous, outputTargetSetId);

	render::handle_t shadowMapAtlasTargetSetId = 0;
	setupLightPass(
		worldRenderView,
		renderGraph,
		outputTargetSetId,
		shadowMapAtlasTargetSetId
	);

	setupVisualPass(
		worldRenderView,
		renderGraph,
		visualTargetSetId.current,
		gbufferTargetSetId,
		dbufferTargetSetId,
		ambientOcclusionTargetSetId,
		contactShadowsTargetSetId,
		reflectionsTargetSetId,
		shadowMapAtlasTargetSetId,
		hizTextureId
	);

	m_postProcessPass->setup(worldRenderView, m_gatheredView, count, renderGraph, gbufferTargetSetId, velocityTargetSetId, visualTargetSetId, outputTargetSetId);

	m_state[worldRenderView.getIndex()].count++;
}

void WorldRendererDeferred::setupVisualPass(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::handle_t visualWriteTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t dbufferTargetSetId,
	render::handle_t ambientOcclusionTargetSetId,
	render::handle_t contactShadowsTargetSetId,
	render::handle_t reflectionsTargetSetId,
	render::handle_t shadowMapAtlasTargetSetId,
	render::handle_t outputHiZTextureId
) const
{
	T_PROFILER_SCOPE(L"World setup visual");

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
	const VolumetricFogComponent* fog = !worldRenderView.getSnapshot() ? m_gatheredView.fog : nullptr;

	// Add visual render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);
	rp->addInput(dbufferTargetSetId);
	rp->addInput(ambientOcclusionTargetSetId);
	// rp->addInput(contactShadowsTargetSetId);
	rp->addInput(reflectionsTargetSetId);
	rp->addInput(shadowMapAtlasTargetSetId);
	rp->addInput(outputHiZTextureId);
	for (auto attachment : m_visualAttachments)
		rp->addInput(attachment);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(visualWriteTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	Ref< render::Buffer > lightSBuffer = m_state[worldRenderView.getIndex()].lightSBuffer;

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				renderContext
			);

			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			const auto dbufferTargetSet = renderGraph.getTargetSet(dbufferTargetSetId);
			const auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
			// const auto contactShadowsTargetSet = renderGraph.getTargetSet(contactShadowsTargetSetId);
			const auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
			const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);

			const auto& view = worldRenderView.getView();
			const auto& projection = worldRenderView.getProjection();

			const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
			const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();
			const float viewSliceScale = ClusterDimZ / std::log(viewFarZ / viewNearZ);
			const float viewSliceBias = ClusterDimZ * std::log(viewNearZ) / std::log(viewFarZ / viewNearZ) - 0.001f;

			const Scalar p11 = projection.get(0, 0);
			const Scalar p22 = projection.get(1, 1);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
			sharedParams->setVectorParameter(s_handleViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
			sharedParams->setVectorParameter(s_handleSlicePositions, Vector4(m_slicePositions[1], m_slicePositions[2], m_slicePositions[3], m_slicePositions[4]));
			sharedParams->setMatrixParameter(s_handleProjection, projection);
			sharedParams->setMatrixParameter(s_handleView, view);
			sharedParams->setMatrixParameter(s_handleViewInverse, view.inverse());
			sharedParams->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));

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
			sharedParams->setBufferViewParameter(s_handleLightSBuffer, lightSBuffer->getBufferView());

			if (probe)
			{
				sharedParams->setFloatParameter(s_handleProbeIntensity, probe->getIntensity());
				sharedParams->setFloatParameter(s_handleProbeTextureMips, (float)probe->getTexture()->getSize().mips);
				sharedParams->setTextureParameter(s_handleProbeTexture, probe->getTexture());
			}
			else
			{
				sharedParams->setFloatParameter(s_handleProbeIntensity, 0.0f);
				sharedParams->setFloatParameter(s_handleProbeTextureMips, 0.0f);
				sharedParams->setTextureParameter(s_handleProbeTexture, m_blackCubeTexture);
			}

			if (fog)
			{
				const Vector4 fogRange(
					viewNearZ,
					std::min< float >(viewFarZ, fog->getMaxDistance()),
					fog->getMaxScattering(),
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

			sharedParams->setTextureParameter(s_handleGBufferA, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleGBufferB, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleGBufferC, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleGBufferD, gbufferTargetSet->getColorTexture(3));

			if (dbufferTargetSet)
			{
				sharedParams->setTextureParameter(s_handleDBufferColorMap, dbufferTargetSet->getColorTexture(0));
				sharedParams->setTextureParameter(s_handleDBufferMiscMap, dbufferTargetSet->getColorTexture(1));
				sharedParams->setTextureParameter(s_handleDBufferNormalMap, dbufferTargetSet->getColorTexture(2));
			}

			if (ambientOcclusionTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleOcclusionMap, m_whiteTexture);

			// if (contactShadowsTargetSet != nullptr)
			// 	sharedParams->setTextureParameter(s_handleContactShadowsMap, contactShadowsTargetSet->getColorTexture(0));
			// else
			// 	sharedParams->setTextureParameter(s_handleContactShadowsMap, m_blackTexture);

			if (reflectionsTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleReflectionMap, reflectionsTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleReflectionMap, m_blackTexture);

			sharedParams->endParameters(renderContext);

			const bool irradianceEnable = (bool)(m_irradianceGrid != nullptr);
			const bool irradianceSingle = irradianceEnable && m_irradianceGrid->isSingle();

			// Analytical lights; resolve with gbuffer.
			{
				render::Shader::Permutation perm;
				m_lightShader->setCombination(s_handleIrradianceEnable, irradianceEnable, perm);
				m_lightShader->setCombination(s_handleIrradianceSingle, irradianceSingle, perm);
				m_lightShader->setCombination(s_handleVolumetricFogEnable, (bool)(fog != nullptr), perm);
				m_screenRenderer->draw(renderContext, m_lightShader, perm, sharedParams, L"GBuffer resolve");
			}

			// Forward visuals; not included in GBuffer.
			const WorldRenderPassShared deferredColorPass(
				s_techniqueDeferredColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::Last,
				{
					{ s_handleIrradianceEnable, irradianceEnable },
					{ s_handleIrradianceSingle, irradianceSingle },
					{ s_handleVolumetricFogEnable, (bool)(fog != nullptr)}
				}
			);

			//T_ASSERT(!renderContext->havePendingDraws());

			for (const auto& r : m_gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, deferredColorPass, r.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, deferredColorPass);
		}
	);

	renderGraph.addPass(rp);
}

}
