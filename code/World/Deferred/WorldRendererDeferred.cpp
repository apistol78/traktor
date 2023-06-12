/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "World/Packer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor::world
{
	namespace
	{

const render::Handle s_handleVisualTargetSet[] =
{
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

const resource::Id< render::Shader > c_lightShader(L"{707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}");
const resource::Id< render::Shader > c_fogShader(L"{9453D74C-76C4-8748-9A5B-9E3D6D4F9406}");

Vector2 jitter(int32_t count)
{
	const Vector2 kernelSize(0.5f, 0.5f);
	return Vector2(
		(float)((count / 2) & 1) * kernelSize.x - kernelSize.x / 2.0f,
		(float)(count & 1) * kernelSize.y - kernelSize.y / 2.0f
	);
}

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

	// Lights struct buffer.
	m_lightSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		LightClusterPass::c_maxLightCount,
		sizeof(LightShaderData),
		true
	);
	if (!m_lightSBuffer)
		return false;

	// Create light, reflection and fog shaders.
	if (!resourceManager->bind(c_lightShader, m_lightShader))
		return false;
	if (!resourceManager->bind(c_fogShader, m_fogShader))
		return false;

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	m_shadowAtlasPacker = new Packer(
		shadowSettings.resolution,
		shadowSettings.resolution
	);

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
	{
		const float ii = float(i) / shadowSettings.cascadingSlices;
		const float log = powf(ii, shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
	}
	m_slicePositions[shadowSettings.cascadingSlices] = shadowSettings.farZ;

	return true;
}

void WorldRendererDeferred::destroy()
{
	WorldRendererShared::destroy();
	safeDestroy(m_lightSBuffer);
	m_lightShader.clear();
	m_fogShader.clear();
	m_irradianceGrid.clear();
}

void WorldRendererDeferred::setup(
	const WorldRenderView& immutableWorldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	WorldRenderView worldRenderView = immutableWorldRenderView;

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
		T_PROFILER_SCOPE(L"WorldRendererDeferred setup extra passes");
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
	rgtd.targets[0].colorFormat = render::TfR16G16B16A16F;
	auto visualReadTargetSetId = renderGraph.addPersistentTargetSet(L"History", s_handleVisualTargetSet[m_count % 2], false, rgtd, m_sharedDepthStencil, outputTargetSetId);
	auto visualWriteTargetSetId = renderGraph.addPersistentTargetSet(L"Visual", s_handleVisualTargetSet[(m_count + 1) % 2], false, rgtd, m_sharedDepthStencil, outputTargetSetId);
	
	// Add passes to render graph.
	m_lightClusterPass->setup(worldRenderView, m_gatheredView);
	auto gbufferTargetSetId = m_gbufferPass->setup(worldRenderView, rootEntity, m_gatheredView, s_techniqueDeferredGBufferWrite, renderGraph, outputTargetSetId);
	auto velocityTargetSetId = m_velocityPass->setup(worldRenderView, rootEntity, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto ambientOcclusionTargetSetId = m_ambientOcclusionPass->setup(worldRenderView, rootEntity, m_gatheredView, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto reflectionsTargetSetId = m_reflectionsPass->setup(worldRenderView, rootEntity, m_gatheredView, renderGraph, gbufferTargetSetId, visualReadTargetSetId, outputTargetSetId);

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

void WorldRendererDeferred::setupLightPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t& outShadowMapAtlasTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererDeferred setupLightPass");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);

	LightShaderData* lightShaderData = (LightShaderData*)m_lightSBuffer->lock();
	if (!lightShaderData)
		return;

	// Reset this frame's atlas packer.
	auto shadowAtlasPacker = m_shadowAtlasPacker;
	shadowAtlasPacker->reset();

	const Matrix44 view = worldRenderView.getView();
	const Matrix44 viewInverse = worldRenderView.getView().inverse();
	const Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Find atlas shadow lights.
	StaticVector< int32_t, 32 > lightAtlasIndices;
	if (shadowsEnable)
	{
		for (int32_t i = 0; i < (int32_t)m_gatheredView.lights.size(); ++i)
		{
			const auto& light = m_gatheredView.lights[i];
			if (
				light != nullptr &&
				light->getCastShadow() &&
				light->getLightType() == LightType::Spot
				)
				lightAtlasIndices.push_back(i);
		}
	}

	// Write all lights to sbuffer; without shadow map information.
	{
		// First 4 entires are cascading shadow light.
		if (m_gatheredView.lights[0] != nullptr)
		{
			const auto& light = m_gatheredView.lights[0];
			auto* lsd = lightShaderData;

			for (int32_t slice = 0; slice < shadowSettings.cascadingSlices; ++slice)
			{
				lsd->typeRangeRadius[0] = (float)light->getLightType();
				lsd->typeRangeRadius[1] = light->getRange();
				lsd->typeRangeRadius[2] = std::cos((light->getRadius() - deg2rad(8.0f)) / 2.0f);
				lsd->typeRangeRadius[3] = std::cos(light->getRadius() / 2.0f);

				const Matrix44 lightTransform = view * light->getTransform().toMatrix44();
				lightTransform.translation().xyz1().storeUnaligned(lsd->position);
				lightTransform.axisY().xyz0().storeUnaligned(lsd->direction);
				(light->getColor() * light->getFlickerCoeff()).storeUnaligned(lsd->color);

				Vector4::zero().storeUnaligned(lsd->viewToLight0);
				Vector4::zero().storeUnaligned(lsd->viewToLight1);
				Vector4::zero().storeUnaligned(lsd->viewToLight2);
				Vector4::zero().storeUnaligned(lsd->viewToLight3);

				++lsd;
			}
		}

		// Append all other lights.
		for (int32_t i = 4; i < (int32_t)m_gatheredView.lights.size(); ++i)
		{
			const auto& light = m_gatheredView.lights[i];
			auto* lsd = &lightShaderData[i];

			lsd->typeRangeRadius[0] = (float)light->getLightType();
			lsd->typeRangeRadius[1] = light->getRange();
			lsd->typeRangeRadius[2] = std::cos((light->getRadius() - deg2rad(8.0f)) / 2.0f);
			lsd->typeRangeRadius[3] = std::cos(light->getRadius() / 2.0f);

			const Matrix44 lightTransform = view * light->getTransform().toMatrix44();
			lightTransform.translation().xyz1().storeUnaligned(lsd->position);
			lightTransform.axisY().xyz0().storeUnaligned(lsd->direction);
			(light->getColor() * light->getFlickerCoeff()).storeUnaligned(lsd->color);

			Vector4::zero().storeUnaligned(lsd->viewToLight0);
			Vector4::zero().storeUnaligned(lsd->viewToLight1);
			Vector4::zero().storeUnaligned(lsd->viewToLight2);
			Vector4::zero().storeUnaligned(lsd->viewToLight3);
		}
	}

	// If shadow casting directional light found add cascade shadow map pass
	// and update light sbuffer.
	if (shadowsEnable)
	{
		const int32_t cascadingSlices = (m_gatheredView.lights[0] != nullptr) ? shadowSettings.cascadingSlices : 0;
		const int32_t shmw = shadowSettings.resolution * (cascadingSlices + 1);
		const int32_t shmh = shadowSettings.resolution;
		const int32_t sliceDim = shadowSettings.resolution;
		const int32_t atlasOffset = shadowSettings.resolution * cascadingSlices;

		// Add shadow map target.
		render::RenderGraphTargetSetDesc rgtd;
		rgtd.count = 0;
		rgtd.width = shmw;
		rgtd.height = shmh;
		rgtd.createDepthStencil = true;
		rgtd.usingPrimaryDepthStencil = false;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		outShadowMapAtlasTargetSetId = renderGraph.addTransientTargetSet(L"Shadow map atlas", rgtd);

		// Add shadow map render passes.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow map");

		render::Clear clear;
		clear.mask = render::CfDepth;
		clear.depth = 1.0f;
		rp->setOutput(outShadowMapAtlasTargetSetId, clear, render::TfNone, render::TfDepth);

		if (m_gatheredView.lights[0] != nullptr)
		{
			const auto& light = m_gatheredView.lights[0];
			const Transform lightTransform = light->getTransform();
			const Vector4 lightPosition = lightTransform.translation().xyz1();
			const Vector4 lightDirection = lightTransform.axisY().xyz0();

			// First 4 entries are allocated to cascading shadow light.
			auto* lsd = lightShaderData;

			for (int32_t slice = 0; slice < shadowSettings.cascadingSlices; ++slice)
			{
				const Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				const Scalar zf(min(m_slicePositions[slice + 1], shadowSettings.farZ));

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

				// Calculate shadow map projection.
				Matrix44 shadowLightView;
				Matrix44 shadowLightProjection;
				Frustum shadowFrustum;

				shadowProjection.calculate(
					viewInverse,
					lightPosition,
					lightDirection,
					sliceViewFrustum,
					shadowSettings.farZ,
					shadowSettings.quantizeProjection,
					shadowLightView,
					shadowLightProjection,
					shadowFrustum
				);

				const Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
				viewToLightSpace.axisX().storeUnaligned(lsd[slice].viewToLight0);
				viewToLightSpace.axisY().storeUnaligned(lsd[slice].viewToLight1);
				viewToLightSpace.axisZ().storeUnaligned(lsd[slice].viewToLight2);
				viewToLightSpace.translation().storeUnaligned(lsd[slice].viewToLight3);

				// Write slice coordinates to shaders.
				Vector4(
					(float)(slice * sliceDim) / shmw,
					0.0f,
					(float)sliceDim / shmw,
					1.0f
				).storeUnaligned(lsd[slice].atlasTransform);

				rp->addBuild(
					[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
					{
						WorldBuildContext wc(
							m_entityRenderers,
							rootEntity,
							renderContext
						);

						// Render shadow map.
						WorldRenderView shadowRenderView;
						shadowRenderView.setProjection(shadowLightProjection);
						shadowRenderView.setView(shadowLightView, shadowLightView);
						shadowRenderView.setViewFrustum(shadowFrustum);
						shadowRenderView.setCullFrustum(shadowFrustum);
						shadowRenderView.setTimes(
							worldRenderView.getTime(),
							worldRenderView.getDeltaTime(),
							worldRenderView.getInterval()
						);

						// Set viewport to current cascade.
						auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
						svrb->viewport = render::Viewport(
							slice * sliceDim,
							0,
							sliceDim,
							sliceDim,
							0.0f,
							1.0f
						);
						renderContext->enqueue(svrb);

						// Render entities into shadow map.
						auto sharedParams = renderContext->alloc< render::ProgramParameters >();
						sharedParams->beginParameters(renderContext);
						sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
						sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
						sharedParams->setMatrixParameter(s_handleView, shadowLightView);
						sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
						sharedParams->endParameters(renderContext);

						const WorldRenderPassShared shadowPass(
							s_techniqueShadow,
							sharedParams,
							shadowRenderView,
							IWorldRenderPass::None
						);

						T_ASSERT(!renderContext->havePendingDraws());

						for (auto r : m_gatheredView.renderables)
							r.renderer->build(wc, shadowRenderView, shadowPass, r.renderable);

						for (auto entityRenderer : m_entityRenderers->get())
							entityRenderer->build(wc, shadowRenderView, shadowPass);
					}
				);
			}
		}

		for (int32_t lightAtlasIndex : lightAtlasIndices)
		{
			const auto& light = m_gatheredView.lights[lightAtlasIndex];
			const Transform lightTransform = light->getTransform();
			const Vector4 lightPosition = lightTransform.translation().xyz1();
			const Vector4 lightDirection = lightTransform.axisY().xyz0();

			auto* lsd = &lightShaderData[lightAtlasIndex];

			// Calculate shadow map projection.
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;
			Frustum shadowFrustum;

			shadowFrustum.buildPerspective(light->getRadius(), 1.0f, 0.1f, light->getRange());
			shadowLightProjection = perspectiveLh(light->getRadius(), 1.0f, 0.1f, light->getRange());

			Vector4 lightAxisX, lightAxisY, lightAxisZ;
			lightAxisZ = -lightDirection;
			lightAxisX = cross(viewInverse.axisZ(), lightAxisZ).normalized();
			lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

			shadowLightView = Matrix44(
				lightAxisX,
				lightAxisY,
				lightAxisZ,
				lightPosition
			);
			shadowLightView = shadowLightView.inverse();

			const Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
			viewToLightSpace.axisX().storeUnaligned(lsd->viewToLight0);
			viewToLightSpace.axisY().storeUnaligned(lsd->viewToLight1);
			viewToLightSpace.axisZ().storeUnaligned(lsd->viewToLight2);
			viewToLightSpace.translation().storeUnaligned(lsd->viewToLight3);

			// Calculate size of shadow region based on distance from eye.
			const float distance = (worldRenderView.getEyePosition() - lightPosition).xyz0().length();
			const int32_t denom = (int32_t)std::floor(distance / 4.0f);
			const int32_t atlasSize = 128 >> std::min(denom, 4);

			Packer::Rectangle atlasRect;
			if (!shadowAtlasPacker->insert(atlasSize, atlasSize, atlasRect))
				return;

			// Write atlas coordinates to shaders.
			Vector4(
				(float)(atlasOffset + atlasRect.x) / shmw,
				(float)atlasRect.y / shmh,
				(float)atlasRect.width / shmw,
				(float)atlasRect.height / shmh
			).storeUnaligned(lsd->atlasTransform);

			rp->addBuild(
				[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
				{
					const WorldBuildContext wc(
						m_entityRenderers,
						rootEntity,
						renderContext
					);

					// Render shadow map.
					WorldRenderView shadowRenderView;
					shadowRenderView.setProjection(shadowLightProjection);
					shadowRenderView.setView(shadowLightView, shadowLightView);
					shadowRenderView.setViewFrustum(shadowFrustum);
					shadowRenderView.setCullFrustum(shadowFrustum);
					shadowRenderView.setTimes(
						worldRenderView.getTime(),
						worldRenderView.getDeltaTime(),
						worldRenderView.getInterval()
					);

					// Set viewport to light atlas slot.
					auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
					svrb->viewport = render::Viewport(
						atlasOffset + atlasRect.x,
						atlasRect.y,
						atlasRect.width,
						atlasRect.height,
						0.0f,
						1.0f
					);
					renderContext->enqueue(svrb);

					// Render entities into shadow map.
					auto sharedParams = renderContext->alloc< render::ProgramParameters >();
					sharedParams->beginParameters(renderContext);
					sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
					sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
					sharedParams->setMatrixParameter(s_handleView, shadowLightView);
					sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
					sharedParams->endParameters(renderContext);

					const WorldRenderPassShared shadowPass(
						s_techniqueShadow,
						sharedParams,
						shadowRenderView,
						IWorldRenderPass::None
					);

					T_ASSERT(!renderContext->havePendingDraws());

					for (auto r : m_gatheredView.renderables)
						r.renderer->build(wc, shadowRenderView, shadowPass, r.renderable);

					for (auto entityRenderer : m_entityRenderers->get())
						entityRenderer->build(wc, shadowRenderView, shadowPass);
				}
			);
		}

		renderGraph.addPass(rp);
	}

	m_lightSBuffer->unlock();
}

void WorldRendererDeferred::setupVisualPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t visualWriteTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t ambientOcclusionTargetSetId,
	render::handle_t reflectionsTargetSetId,
	render::handle_t shadowMapAtlasTargetSetId
) const
{
	T_PROFILER_SCOPE(L"World setup visual");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

	// Find first, non-local, probe.
	const ProbeComponent* probe = nullptr;
	for (auto p : m_gatheredView.probes)
	{
		if (!p->getLocal() && p->getTexture() != nullptr)
		{
			probe = p;
			break;
		}
	}

	// Add visual render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);
	rp->addInput(ambientOcclusionTargetSetId);
	rp->addInput(reflectionsTargetSetId);
	rp->addInput(shadowMapAtlasTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(visualWriteTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			const auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
			const auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
			const auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);

			const auto& view = worldRenderView.getView();
			const auto& projection = worldRenderView.getProjection();

			const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
			const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();

			const Scalar p11 = projection.get(0, 0);
			const Scalar p22 = projection.get(1, 1);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
			sharedParams->setVectorParameter(s_handleViewDistance, Vector4(viewNearZ, viewFarZ, 0.0f, 0.0f));
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
			sharedParams->setBufferViewParameter(s_handleLightSBuffer, m_lightSBuffer->getBufferView());

			sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, Vector4(m_settings.fogDistance, m_settings.fogDensity, 0.0f, 0.0f));
			sharedParams->setVectorParameter(s_handleFogColor, m_settings.fogColor);

			if (shadowAtlasTargetSet != nullptr)
			{
				sharedParams->setFloatParameter(s_handleShadowBias, shadowSettings.bias);
				sharedParams->setTextureParameter(s_handleShadowMapAtlas, shadowAtlasTargetSet->getDepthTexture());
			}

			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleColorMap, gbufferTargetSet->getColorTexture(2));

			if (ambientOcclusionTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleOcclusionMap, m_whiteTexture);

			if (reflectionsTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleReflectionMap, reflectionsTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleReflectionMap, m_blackTexture);

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

			sharedParams->endParameters(renderContext);

			// Irradiance
			const WorldRenderPassShared irradiancePass(
				s_techniqueIrradianceWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::None
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto r : m_gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, irradiancePass, r.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, irradiancePass);

			// Do a manual merge so irradiance is written always.
			renderContext->mergeDraw(render::RpAll);

			// Analytical lights; resolve with gbuffer.
			{
				render::Shader::Permutation perm;
				m_lightShader->setCombination(s_handleShadowEnable, (bool)(shadowAtlasTargetSet != nullptr), perm);
				m_lightShader->setCombination(s_handleIrradianceEnable, (bool)(m_irradianceGrid != nullptr), perm);
				m_lightShader->setCombination(s_handleVolumetricFogEnable, false, perm);
				m_screenRenderer->draw(renderContext, m_lightShader, perm, sharedParams);
			}

			// Forward visuals; not included in GBuffer.
			const WorldRenderPassShared deferredColorPass(
				s_techniqueDeferredColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::Last
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto r : m_gatheredView.renderables)
				r.renderer->build(wc, worldRenderView, deferredColorPass, r.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, deferredColorPass);
		}
	);

	renderGraph.addPass(rp);
}

}
