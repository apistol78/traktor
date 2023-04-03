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

	// Pack fog parameters.
	m_fogDistanceAndDensity = Vector4(m_settings.fogDistance, m_settings.fogDensity, 0.0f, 0.0f);
	m_fogColor = m_settings.fogColor;

	// Create light, reflection and fog shaders.
	if (!resourceManager->bind(c_lightShader, m_lightShader))
		return false;
	if (!resourceManager->bind(c_fogShader, m_fogShader))
		return false;

	// Create shadow screen projection processes.
	if (!m_shadowSettings.maskProject.isNull() && m_shadowsQuality > Quality::Disabled)
	{
		if (!resourceManager->bind(m_shadowSettings.maskProject, m_shadowMaskProject))
		{
			log::warning << L"Unable to create shadow project process; shadows disabled." << Endl;
			m_shadowsQuality = Quality::Disabled;
		}
	}
	else
		m_shadowsQuality = Quality::Disabled;

	// Create shadow maps.
	if (m_shadowsQuality > Quality::Disabled)
	{
		const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
		render::RenderTargetSetCreateDesc rtscd;

		// Cascading shadow map.
		rtscd.count = 0;
		rtscd.width = shadowSettings.resolution;
		rtscd.height = shadowSettings.cascadingSlices * shadowSettings.resolution;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		m_shadowMapCascadeTargetSet = renderSystem->createRenderTargetSet(rtscd, nullptr, T_FILE_LINE_W);

		// Atlas shadow map.
		rtscd.count = 0;
		rtscd.width =
		rtscd.height = 4096;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		m_shadowMapAtlasTargetSet = renderSystem->createRenderTargetSet(rtscd, nullptr, T_FILE_LINE_W);
	}

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
	{
		const float ii = float(i) / m_shadowSettings.cascadingSlices;
		const float log = powf(ii, m_shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, m_shadowSettings.farZ, log);
	}
	m_slicePositions[m_shadowSettings.cascadingSlices] = m_shadowSettings.farZ;

	return true;
}

void WorldRendererDeferred::destroy()
{
	WorldRendererShared::destroy();

	safeDestroy(m_shadowMapCascadeTargetSet);
	safeDestroy(m_shadowMapAtlasTargetSet);
	safeDestroy(m_lightSBuffer);

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
	// if (m_antiAliasQuality >= Quality::Ultra)
	// {
	// 	Vector2 r = (jitter(m_count) * 2.0f) / worldRenderView.getViewSize();
	// 	Matrix44 proj = immutableWorldRenderView.getProjection();
	// 	proj = translate(r.x, r.y, 0.0f) * proj;
	// 	worldRenderView.setProjection(proj);
	// }

	// Gather active renderables for this frame.
	gather(const_cast< Entity* >(rootEntity));

	// // Begun writing light shader data; written both in setup and build.
	// LightShaderData* lightShaderData = (LightShaderData*)m_lightSBufferMemory;
	// TileShaderData* tileShaderData = (TileShaderData*)m_tileSBufferMemory;

	// // Write all lights to sbuffer; without shadow map information.
	// const Matrix44& view = worldRenderView.getView();
	// for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
	// {
	// 	const auto light = m_lights[i];
	// 	auto* lsd = &lightShaderData[i];

	// 	lsd->typeRangeRadius[0] = (float)light->getLightType();
	// 	lsd->typeRangeRadius[1] = light->getRange();
	// 	lsd->typeRangeRadius[2] = std::cos((light->getRadius() - deg2rad(8.0f)) / 2.0f);
	// 	lsd->typeRangeRadius[3] = std::cos(light->getRadius() / 2.0f);

	// 	Transform lightTransform = Transform(view) * light->getTransform();
	// 	lightTransform.translation().xyz1().storeUnaligned(lsd->position);
	// 	lightTransform.axisY().xyz0().storeUnaligned(lsd->direction);
	// 	light->getColor().storeUnaligned(lsd->color);

	// 	Vector4::zero().storeUnaligned(lsd->viewToLight0);
	// 	Vector4::zero().storeUnaligned(lsd->viewToLight1);
	// 	Vector4::zero().storeUnaligned(lsd->viewToLight2);
	// 	Vector4::zero().storeUnaligned(lsd->viewToLight3);
	// }

	// Find directional light for cascade shadow map.
	int32_t lightCascadeIndex = -1;
	if (m_shadowsQuality != Quality::Disabled)
	{
		for (int32_t i = 0; i < (int32_t)m_gatheredView.lights.size(); ++i)
		{
			const auto light = m_gatheredView.lights[i];
			if (light->getCastShadow() && light->getLightType() == LightType::Directional)
			{
				lightCascadeIndex = i;
				break;
			}
		}
	}

	// Find spot lights for atlas shadow map.
	StaticVector< int32_t, 16 > lightAtlasIndices;
	if (m_shadowsQuality != Quality::Disabled)
	{
		for (int32_t i = 0; i < (int32_t)m_gatheredView.lights.size(); ++i)
		{
			const auto light = m_gatheredView.lights[i];
			if (light->getCastShadow() && light->getLightType() == LightType::Spot)
				lightAtlasIndices.push_back(i);
		}
	}

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererDeferred setup extra passes");
		WorldSetupContext context(m_entityRenderers, rootEntity, renderGraph);

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
	auto visualReadTargetSetId = renderGraph.addPersistentTargetSet(L"History", s_handleVisualTargetSet[m_count % 2], rgtd, m_sharedDepthStencil, outputTargetSetId);
	auto visualWriteTargetSetId = renderGraph.addPersistentTargetSet(L"Visual", s_handleVisualTargetSet[(m_count + 1) % 2], rgtd, m_sharedDepthStencil, outputTargetSetId);
	
	// Add passes to render graph.
	m_lightClusterPass->setup(worldRenderView, m_gatheredView);
	auto gbufferTargetSetId = m_gbufferPass->setup(worldRenderView, rootEntity, m_gatheredView, renderGraph, outputTargetSetId);
	auto velocityTargetSetId = m_velocityPass->setup(worldRenderView, rootEntity, m_gatheredView, m_imageGraphContext, renderGraph, gbufferTargetSetId, outputTargetSetId);
	auto ambientOcclusionTargetSetId = m_ambientOcclusionPass->setup(worldRenderView, rootEntity, m_gatheredView, m_imageGraphContext, renderGraph, gbufferTargetSetId, outputTargetSetId);

	auto shadowMapCascadeTargetSetId = (int32_t)0;
	// setupCascadeShadowMapPass(
	// 	worldRenderView,
	// 	rootEntity,
	// 	renderGraph,
	// 	outputTargetSetId,
	// 	lightCascadeIndex,
	// 	lightShaderData
	// );

	auto shadowMapAtlasTargetSetId = (int32_t)0;
	// setupAtlasShadowMapPass(
	// 	worldRenderView,
	// 	rootEntity,
	// 	renderGraph,
	// 	outputTargetSetId,
	// 	lightAtlasIndices,
	// 	lightShaderData
	// );

	auto shadowMaskTargetSetId = setupShadowMaskPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		shadowMapCascadeTargetSetId,
		lightCascadeIndex
	);

	auto reflectionsTargetSetId = m_reflectionsPass->setup(worldRenderView, rootEntity, m_gatheredView, m_imageGraphContext, renderGraph, gbufferTargetSetId, visualReadTargetSetId, outputTargetSetId);

	setupVisualPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		visualWriteTargetSetId,
		gbufferTargetSetId,
		ambientOcclusionTargetSetId,
		reflectionsTargetSetId,
		shadowMaskTargetSetId,
		shadowMapAtlasTargetSetId
	);

	m_postProcessPass->setup(worldRenderView, rootEntity, m_gatheredView, m_imageGraphContext, renderGraph, gbufferTargetSetId, velocityTargetSetId, visualWriteTargetSetId, visualReadTargetSetId, outputTargetSetId);

	m_count++;
}

render::handle_t WorldRendererDeferred::setupCascadeShadowMapPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	int32_t lightCascadeIndex,
	LightShaderData* lightShaderData
) const
{
	if (lightCascadeIndex < 0)
		return 0;

	T_PROFILER_SCOPE(L"World setup cascade shadow map");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Add cascading shadow map target.
	auto shadowMapCascadeTargetSetId = renderGraph.addTargetSet(L"Shadow map cascade", m_shadowMapCascadeTargetSet);

	// Add cascading shadow map render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow cascade");

	render::Clear clear;
	clear.mask = render::CfDepth;
	clear.depth = 1.0f;
	rp->setOutput(shadowMapCascadeTargetSetId, clear, render::TfNone, render::TfDepth);

	const auto light = m_gatheredView.lights[lightCascadeIndex];
	Transform lightTransform = Transform(view) * light->getTransform();
	Vector4 lightPosition = lightTransform.translation().xyz1();
	Vector4 lightDirection = lightTransform.axisY().xyz0();

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto* lsd = &lightShaderData[lightCascadeIndex];

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
					0,
					slice * shadowSettings.resolution,
					shadowSettings.resolution,
					shadowSettings.resolution,
					0.0f,
					1.0f
				);
				renderContext->enqueue(svrb);	

				// Render entities into shadow map.
				auto sharedParams = renderContext->alloc< render::ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
				sharedParams->setMatrixParameter(s_handleView, shadowLightView);
				sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
				sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
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
		}
	);

	renderGraph.addPass(rp);
	return shadowMapCascadeTargetSetId;
}

render::handle_t WorldRendererDeferred::setupAtlasShadowMapPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	const StaticVector< int32_t, 16 >& lightAtlasIndices,
	LightShaderData* lightShaderData
) const
{
	if (lightAtlasIndices.empty())
		return 0;

	T_PROFILER_SCOPE(L"World setup atlas shadow map");

	const auto shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Add atlas shadow map target.
	auto shadowMapAtlasTargetSetId = renderGraph.addTargetSet(L"Shadow map atlas", m_shadowMapAtlasTargetSet);

	// Add atlas shadow map render pass.
	int32_t atlasIndex = 0;
	for (int32_t lightAtlasIndex : lightAtlasIndices)
	{
		const auto light = m_gatheredView.lights[lightAtlasIndex];
		Transform lightTransform = Transform(view) * light->getTransform();
		Vector4 lightPosition = lightTransform.translation().xyz1();
		Vector4 lightDirection = lightTransform.axisY().xyz0();

		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow atlas");

		render::Clear clear;
		clear.mask = render::CfDepth;
		clear.depth = 1.0f;
		rp->setOutput(shadowMapAtlasTargetSetId, clear, render::TfDepth, render::TfDepth);

		rp->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				WorldBuildContext wc(
					m_entityRenderers,
					rootEntity,
					renderContext
				);

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
					(atlasIndex & 3) * 1024,
					(atlasIndex / 4) * 1024,
					1024,
					1024,
					0.0f,
					1.0f
				);
				renderContext->enqueue(svrb);	

				// Render entities into shadow map.
				auto sharedParams = renderContext->alloc< render::ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
				sharedParams->setMatrixParameter(s_handleView, shadowLightView);
				sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
				sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
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

				// Write transposed matrix to shaders as shaders have row-major order.
				Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
				Matrix44 vls = viewToLightSpace.transpose();
				vls.axisX().storeUnaligned(lsd->viewToLight0);
				vls.axisY().storeUnaligned(lsd->viewToLight1);
				vls.axisZ().storeUnaligned(lsd->viewToLight2);
				vls.translation().storeUnaligned(lsd->viewToLight3);

				// Write atlas coordinates to shaders.
				Vector4(
					(atlasIndex & 3) / 4.0f,
					(atlasIndex / 4) / 4.0f,
					1.0f / 4.0f,
					1.0f / 4.0f
				).storeUnaligned(lsd->atlasTransform);					
			}
		);

		renderGraph.addPass(rp);

		++atlasIndex;
	}

	return shadowMapAtlasTargetSetId;
}

render::handle_t WorldRendererDeferred::setupShadowMaskPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t shadowMapCascadeTargetSetId,
	int32_t lightCascadeIndex
) const
{
	if (m_shadowsQuality == Quality::Disabled || lightCascadeIndex < 0)
		return 0;

	T_PROFILER_SCOPE(L"World setup shadow mask");

	const auto shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);
	const auto light = m_gatheredView.lights[lightCascadeIndex];

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Add screen space shadow mask target.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.width = 0;
	rgtd.height = 0;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.ignoreStencil = true;
	rgtd.targets[0].colorFormat = render::TfR8;
	rgtd.referenceWidthDenom = m_shadowSettings.maskDenominator;
	rgtd.referenceHeightDenom = m_shadowSettings.maskDenominator;
	auto shadowMaskTargetSetId = renderGraph.addTransientTargetSet(L"Shadow mask", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add screen space shadow mask render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow mask");

	Transform lightTransform = Transform(view) * light->getTransform();
	Vector4 lightPosition = lightTransform.translation().xyz1();
	Vector4 lightDirection = lightTransform.axisY().xyz0();

	// Add sub-pass for each slice.
	for (int32_t slice = 0; slice < m_shadowSettings.cascadingSlices; ++slice)
	{
		Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
		Scalar zf(min(m_slicePositions[slice + 1], m_shadowSettings.farZ));

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

		render::ImageGraphView view;
		view.viewFrustum = worldRenderView.getViewFrustum();
		view.viewToLight = shadowLightProjection * shadowLightView * viewInverse;
		view.projection = worldRenderView.getProjection();
		view.sliceCount = m_shadowSettings.cascadingSlices;
		view.sliceIndex = slice;
		view.sliceNearZ = zn;
		view.sliceFarZ = zf;
		view.shadowFarZ = m_shadowSettings.farZ;
		view.shadowMapBias = m_shadowSettings.bias + slice * m_shadowSettings.biasCoeff;
		view.shadowMapUvTransform = Vector4(
			0.0f, (float)slice / m_shadowSettings.cascadingSlices,
			1.0f, 1.0f / m_shadowSettings.cascadingSlices
		);
		view.deltaTime = 0.0f;
		view.time = 0.0f;

		render::ImageGraphContext context;
		context.associateTextureTargetSetDepth(s_handleInputShadowMap, shadowMapCascadeTargetSetId);
		context.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
		context.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);

		m_shadowMaskProject->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			context,
			view
		);
	}

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(shadowMaskTargetSetId, clear, render::TfNone, render::TfColor);

	renderGraph.addPass(rp);
	return shadowMaskTargetSetId;
}

void WorldRendererDeferred::setupVisualPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t visualWriteTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t ambientOcclusionTargetSetId,
	render::handle_t reflectionsTargetSetId,
	render::handle_t shadowMaskTargetSetId,
	render::handle_t shadowMapAtlasTargetSetId
) const
{
	T_PROFILER_SCOPE(L"World setup visual");

	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);
	const int32_t lightCount = (int32_t)m_gatheredView.lights.size();

	// Add visual[0] render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);
	rp->addInput(ambientOcclusionTargetSetId);
	rp->addInput(reflectionsTargetSetId);
	rp->addInput(shadowMaskTargetSetId);
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

			auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
			auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
			auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
			auto shadowMaskTargetSet = renderGraph.getTargetSet(shadowMaskTargetSetId);
			auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);

			const auto& view = worldRenderView.getView();
			const auto& projection = worldRenderView.getProjection();

			const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
			const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();

			const Scalar p11 = projection.get(0, 0);
			const Scalar p22 = projection.get(1, 1);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setFloatParameter(s_handleLightCount, (float)lightCount);
			sharedParams->setVectorParameter(s_handleViewDistance, Vector4(viewNearZ, viewFarZ, 0.0f, 0.0f));
			sharedParams->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
			sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, m_fogDistanceAndDensity);
			sharedParams->setVectorParameter(s_handleFogColor, m_fogColor);
			sharedParams->setMatrixParameter(s_handleView, view);
			sharedParams->setMatrixParameter(s_handleViewInverse, view.inverse());
			sharedParams->setMatrixParameter(s_handleProjection, projection);
			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleColorMap, gbufferTargetSet->getColorTexture(3));
			sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleShadowMask, (shadowMaskTargetSet != nullptr) ? shadowMaskTargetSet->getColorTexture(0) : m_whiteTexture.ptr());
			sharedParams->setTextureParameter(s_handleShadowMapAtlas, (shadowAtlasTargetSet != nullptr) ? shadowAtlasTargetSet->getDepthTexture() : m_whiteTexture.ptr());
			sharedParams->setTextureParameter(s_handleReflectionMap, (reflectionsTargetSet != nullptr) ? reflectionsTargetSet->getColorTexture(0) : m_whiteTexture.ptr());
			sharedParams->setBufferViewParameter(s_handleLightSBuffer, m_lightSBuffer->getBufferView());
			sharedParams->setBufferViewParameter(s_handleTileSBuffer, m_lightClusterPass->getTileSBuffer()->getBufferView());
			if (m_irradianceGrid)
			{
				const auto size = m_irradianceGrid->getSize();
				sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
				sharedParams->setBufferViewParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer()->getBufferView());
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
			render::Shader::Permutation perm;
			m_lightShader->setCombination(s_handleShadowEnable, (bool)(shadowMaskTargetSet != nullptr), perm);
			m_lightShader->setCombination(s_handleIrradianceEnable, (bool)(m_irradianceGrid != nullptr), perm);
			m_screenRenderer->draw(renderContext, m_lightShader, perm, sharedParams);

			// Module with fog.
			if (dot4(m_fogDistanceAndDensity, Vector4(0.0f, 0.0f, 1.0f, 1.0f)) > FUZZY_EPSILON)
				m_screenRenderer->draw(renderContext, m_fogShader, sharedParams);

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
