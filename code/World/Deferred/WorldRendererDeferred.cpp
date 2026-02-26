/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Deferred/WorldRendererDeferred.h"

#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
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
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/Entity/FogComponent.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/ContactShadowsPass.h"
#include "World/Shared/Passes/DBufferPass.h"
#include "World/Shared/Passes/DownScalePass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/HiZPass.h"
#include "World/Shared/Passes/IrradiancePass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/RTReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <limits>

namespace traktor::world
{
namespace
{

const render::Handle s_persistentVisualTargetSet[] = {
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

const resource::Id< render::Shader > c_lightShader(L"{707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}");
const resource::Id< render::Shader > c_copyShader(L"{06BE4DF8-8D5D-8246-805E-B2A70B6DDB66}");

Random s_random;

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, WorldRendererShared)

bool WorldRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc)
{
	if (!WorldRendererShared::create(
			resourceManager,
			renderSystem,
			desc))
		return false;

	// Create light, reflection and fog shaders.
	if (!resourceManager->bind(c_lightShader, m_lightShader))
		return false;

	// Create copy shader.
	if (!resourceManager->bind(c_copyShader, m_copyShader))
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
	render::RGTargetSet outputTargetSetId,
	const std::function< bool(const EntityState& state) >& filter)
{
	WorldRenderView worldRenderView = immutableWorldRenderView;
	const uint32_t count = m_state[worldRenderView.getIndex()].count;
	const bool needJitter = m_postProcessPass->needCameraJitter();
	render::RenderGraphTargetSetDesc rgtsd;

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
		T_PROFILER_SCOPE(L"WorldRendererDeferred setup extra passes");
		WorldSetupContext context(world, m_entityRenderers, renderGraph, m_visualAttachments);

		for (const auto& r : m_gatheredView.renderables)
			r.renderer->setup(context, worldRenderView, r.renderable);

		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add visual target sets.
	rgtsd = render::RenderGraphTargetSetDesc();
	rgtsd.count = 1;
	rgtsd.createDepthStencil = false;
	rgtsd.referenceWidthDenom = 1;
	rgtsd.referenceHeightDenom = 1;
	rgtsd.targets[0].colorFormat = render::TfR16G16B16A16F;
	const DoubleBufferedTarget visualTargetSetId = {
		renderGraph.addPersistentTargetSet(L"Previous", s_persistentVisualTargetSet[count % 2], false, rgtsd, outputTargetSetId, outputTargetSetId),
		renderGraph.addPersistentTargetSet(L"Current", s_persistentVisualTargetSet[(count + 1) % 2], false, rgtsd, outputTargetSetId, outputTargetSetId)
	};

	rgtsd = render::RenderGraphTargetSetDesc();
	rgtsd.count = 1;
	rgtsd.createDepthStencil = false;
	rgtsd.referenceWidthDenom = 1;
	rgtsd.referenceHeightDenom = 1;
	rgtsd.targets[0].colorFormat = render::TfR16G16B16A16F;
	const render::RGTargetSet visualCopyTargetSetId = renderGraph.addTransientTargetSet(L"Visual Copy", rgtsd, render::RGTargetSet::Invalid, outputTargetSetId);

	// Add Hi-Z texture.
	const render::RGTexture hizTextureId = m_hiZPass->addTexture(worldRenderView, renderGraph);

	render::RGTargetSet shadowMapAtlasTargetSetId;
	setupLightPass(
		worldRenderView,
		renderGraph,
		outputTargetSetId,
		shadowMapAtlasTargetSetId);

	const render::Buffer* lightSBuffer = m_state[worldRenderView.getIndex()].lightSBuffer;

	// Add passes to render graph.
	m_lightClusterPass->setup(worldRenderView, m_gatheredView);
	const auto gbufferTargetSetId = m_gbufferPass->setup(worldRenderView, m_gatheredView, ShaderTechnique::DeferredGBufferWrite, renderGraph, hizTextureId, outputTargetSetId);
	const auto dbufferTargetSetId = m_dbufferPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	const auto halfResDepthTextureId = m_downScalePass->setup(worldRenderView, renderGraph, gbufferTargetSetId);
	m_hiZPass->setup(worldRenderView, renderGraph, gbufferTargetSetId, hizTextureId);
	const auto velocityTargetSetId = m_velocityPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	const auto irradianceTargetSetId = m_irradiancePass->setup(worldRenderView, m_gatheredView, lightSBuffer, needJitter, count, renderGraph, gbufferTargetSetId, velocityTargetSetId, halfResDepthTextureId, outputTargetSetId);
	const auto ambientOcclusionTargetSetId = m_ambientOcclusionPass->setup(worldRenderView, m_gatheredView, needJitter, count, renderGraph, gbufferTargetSetId, halfResDepthTextureId, outputTargetSetId);
	const auto contactShadowsTargetSetId = m_contactShadowsPass->setup(worldRenderView, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	const auto reflectionsTargetSetId = m_reflectionsPass->setup(worldRenderView, m_gatheredView, lightSBuffer, m_blackCubeTexture, needJitter, count, renderGraph, gbufferTargetSetId, dbufferTargetSetId, visualTargetSetId.previous, velocityTargetSetId, halfResDepthTextureId, outputTargetSetId);

	setupVisualPass(
		worldRenderView,
		renderGraph,
		visualTargetSetId.current,
		visualCopyTargetSetId,
		gbufferTargetSetId,
		dbufferTargetSetId,
		irradianceTargetSetId,
		ambientOcclusionTargetSetId,
		contactShadowsTargetSetId,
		reflectionsTargetSetId,
		shadowMapAtlasTargetSetId,
		hizTextureId);

	m_postProcessPass->setup(worldRenderView, m_gatheredView, count, m_whiteTexture, renderGraph, gbufferTargetSetId, velocityTargetSetId, visualTargetSetId, outputTargetSetId);

	m_state[worldRenderView.getIndex()].count++;
}

void WorldRendererDeferred::setupVisualPass(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet visualWriteTargetSetId,
	render::RGTargetSet visualCopyTargetSetId,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet dbufferTargetSetId,
	render::RGTargetSet irradianceTargetSetId,
	render::RGTargetSet ambientOcclusionTargetSetId,
	render::RGTargetSet contactShadowsTargetSetId,
	render::RGTargetSet reflectionsTargetSetId,
	render::RGTargetSet shadowMapAtlasTargetSetId,
	render::RGTexture outputHiZTextureId) const
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
	const FogComponent* fog = m_gatheredView.fog;

	// Calculate jitter vectors.
	const uint32_t count = m_state[worldRenderView.getIndex()].count;
	const bool needJitter = m_postProcessPass->needCameraJitter();
	const Vector2 jrc = needJitter ? jitter(count) / worldRenderView.getViewSize() : Vector2::zero();
	const Vector2 jrp = needJitter ? jitter(count - 1) / worldRenderView.getViewSize() : Vector2::zero();

	// Resolve GBuffer to visual target.
	{
		// Add visual render pass.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Visual; opaque");
		rp->addInput(gbufferTargetSetId);
		rp->addInput(dbufferTargetSetId);
		rp->addInput(irradianceTargetSetId);
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
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			const WorldBuildContext wc(
				m_entityRenderers,
				renderContext);

			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			const auto dbufferTargetSet = renderGraph.getTargetSet(dbufferTargetSetId);
			const auto irradianceTargetSet = renderGraph.getTargetSet(irradianceTargetSetId);
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
			sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
			sharedParams->setFloatParameter(ShaderParameter::Random, s_random.nextFloat());
			sharedParams->setVectorParameter(ShaderParameter::Jitter, Vector4(jrp.x, -jrp.y, jrc.x, -jrc.y)); // Texture space.
			sharedParams->setVectorParameter(ShaderParameter::ViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
			sharedParams->setVectorParameter(ShaderParameter::SlicePositions, Vector4(m_slicePositions[1], m_slicePositions[2], m_slicePositions[3], m_slicePositions[4]));
			sharedParams->setMatrixParameter(ShaderParameter::Projection, projection);
			sharedParams->setMatrixParameter(ShaderParameter::View, view);
			sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, view.inverse());
			sharedParams->setVectorParameter(ShaderParameter::MagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));

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

			if (dbufferTargetSet)
			{
				sharedParams->setTextureParameter(ShaderParameter::DBufferColorMap, dbufferTargetSet->getColorTexture(0));
				sharedParams->setTextureParameter(ShaderParameter::DBufferMiscMap, dbufferTargetSet->getColorTexture(1));
				sharedParams->setTextureParameter(ShaderParameter::DBufferNormalMap, dbufferTargetSet->getColorTexture(2));
			}

			if (irradianceTargetSet != nullptr)
				sharedParams->setTextureParameter(ShaderParameter::IrradianceMap, irradianceTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(ShaderParameter::IrradianceMap, m_whiteTexture);

			if (ambientOcclusionTargetSet != nullptr)
				sharedParams->setTextureParameter(ShaderParameter::OcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(ShaderParameter::OcclusionMap, m_whiteTexture);

			// if (contactShadowsTargetSet != nullptr)
			// 	sharedParams->setTextureParameter(ShaderParameter::ContactShadowsMap, contactShadowsTargetSet->getColorTexture(0));
			// else
			// 	sharedParams->setTextureParameter(ShaderParameter::ContactShadowsMap, m_blackTexture);

			if (reflectionsTargetSet != nullptr)
				sharedParams->setTextureParameter(ShaderParameter::ReflectionMap, reflectionsTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(ShaderParameter::ReflectionMap, m_blackTexture);

			if (m_gatheredView.rtWorldTopLevel != nullptr)
				sharedParams->setAccelerationStructureParameter(ShaderParameter::TLAS, m_gatheredView.rtWorldTopLevel);

			sharedParams->endParameters(renderContext);

			const bool irradianceEnable = (bool)(m_gatheredView.irradianceGrid != nullptr);
			const bool irradianceSingle = irradianceEnable && m_gatheredView.irradianceGrid->isSingle();

			// Analytical lights; resolve with gbuffer.
			{
				render::Shader::Permutation perm;
				m_lightShader->setCombination(ShaderPermutation::IrradianceEnable, irradianceEnable, perm);
				m_lightShader->setCombination(ShaderPermutation::IrradianceSingle, irradianceSingle, perm);
				m_lightShader->setCombination(ShaderPermutation::VolumetricFogEnable, (bool)(fog != nullptr && fog->m_volumetricFogEnable), perm);
				m_lightShader->setCombination(ShaderPermutation::RayTracingEnable, (bool)(m_gatheredView.rtWorldTopLevel != nullptr), perm);
				m_screenRenderer->draw(renderContext, m_lightShader, perm, sharedParams, L"GBuffer resolve");
			}
		});

		renderGraph.addPass(rp);
	}

	// Copy visual target to be used for refraction etc.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Visual; copy");
		rp->addInput(visualWriteTargetSetId);
		rp->setOutput(visualCopyTargetSetId, render::TfNone, render::TfColor);
		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			const auto visualWriteTargetSet = renderGraph.getTargetSet(visualWriteTargetSetId);

			auto params = renderContext->alloc< render::ProgramParameters >();
			params->beginParameters(renderContext);
			params->setTextureParameter(render::getParameterHandle(L"Source"), visualWriteTargetSet->getColorTexture(0));
			params->endParameters(renderContext);

			m_screenRenderer->draw(renderContext, m_copyShader, params, L"Copy");
		});

		renderGraph.addPass(rp);
	}

	// Forward visuals; transparent etc.
	{
		// Add visual render pass.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Visual; non deferred");
		rp->addInput(gbufferTargetSetId);
		rp->addInput(dbufferTargetSetId);
		rp->addInput(ambientOcclusionTargetSetId);
		// rp->addInput(contactShadowsTargetSetId);
		rp->addInput(reflectionsTargetSetId);
		rp->addInput(shadowMapAtlasTargetSetId);
		rp->addInput(outputHiZTextureId);
		rp->addInput(visualCopyTargetSetId);
		for (auto attachment : m_visualAttachments)
			rp->addInput(attachment);

		rp->setOutput(visualWriteTargetSetId, render::TfColor | render::TfDepth, render::TfColor | render::TfDepth);

		Ref< render::Buffer > lightSBuffer = m_state[worldRenderView.getIndex()].lightSBuffer;

		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			const WorldBuildContext wc(
				m_entityRenderers,
				renderContext);

			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			const auto dbufferTargetSet = renderGraph.getTargetSet(dbufferTargetSetId);
			const auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
			// const auto contactShadowsTargetSet = renderGraph.getTargetSet(contactShadowsTargetSetId);
			const auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
			const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);
			const auto visualCopyTargetSet = renderGraph.getTargetSet(visualCopyTargetSetId);

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
			sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
			sharedParams->setVectorParameter(ShaderParameter::ViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
			sharedParams->setVectorParameter(ShaderParameter::SlicePositions, Vector4(m_slicePositions[1], m_slicePositions[2], m_slicePositions[3], m_slicePositions[4]));
			sharedParams->setMatrixParameter(ShaderParameter::Projection, projection);
			sharedParams->setMatrixParameter(ShaderParameter::View, view);
			sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, view.inverse());
			sharedParams->setVectorParameter(ShaderParameter::MagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));

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

			if (dbufferTargetSet)
			{
				sharedParams->setTextureParameter(ShaderParameter::DBufferColorMap, dbufferTargetSet->getColorTexture(0));
				sharedParams->setTextureParameter(ShaderParameter::DBufferMiscMap, dbufferTargetSet->getColorTexture(1));
				sharedParams->setTextureParameter(ShaderParameter::DBufferNormalMap, dbufferTargetSet->getColorTexture(2));
			}

			if (ambientOcclusionTargetSet != nullptr)
				sharedParams->setTextureParameter(ShaderParameter::OcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(ShaderParameter::OcclusionMap, m_whiteTexture);

			// if (contactShadowsTargetSet != nullptr)
			// 	sharedParams->setTextureParameter(ShaderParameter::ContactShadowsMap, contactShadowsTargetSet->getColorTexture(0));
			// else
			// 	sharedParams->setTextureParameter(ShaderParameter::ContactShadowsMap, m_blackTexture);

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

			sharedParams->endParameters(renderContext);

			const bool irradianceEnable = (bool)(m_gatheredView.irradianceGrid != nullptr);
			const bool irradianceSingle = irradianceEnable && m_gatheredView.irradianceGrid->isSingle();

			const WorldRenderPassShared deferredColorPass(
				ShaderTechnique::DeferredColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::Last,
				{ { ShaderPermutation::IrradianceEnable, irradianceEnable },
					{ ShaderPermutation::IrradianceSingle, irradianceSingle },
					{ ShaderPermutation::VolumetricFogEnable, (bool)(fog != nullptr && fog->m_volumetricFogEnable) } });

			for (const auto& r : m_gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, deferredColorPass, r.renderable);

			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, deferredColorPass);
		});

		renderGraph.addPass(rp);
	}
}

}
