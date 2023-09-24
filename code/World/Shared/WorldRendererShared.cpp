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
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/Packer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldGatherContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/VolumetricFogComponent.h"
#include "World/Shared/WorldRendererShared.h"
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

const resource::Id< render::Shader > c_clearDepthShader(L"{0135F7CC-FC65-4FD9-BBD5-CCE0C003B540}");

Ref< render::ITexture > create1x1Texture(render::IRenderSystem* renderSystem, uint32_t value)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 1;
	stcd.height = 1;
	stcd.mipCount = 1;
	stcd.format = render::TfR8G8B8A8;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = &value;
	stcd.initialData[0].pitch = 4;
	return renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
}

Ref< render::ITexture > createCubeTexture(render::IRenderSystem* renderSystem, uint32_t value)
{
	render::CubeTextureCreateDesc ctcd = {};
	ctcd.side = 1;
	ctcd.mipCount = 1;
	ctcd.format = render::TfR8G8B8A8;
	ctcd.sRGB = false;
	ctcd.immutable = true;
	ctcd.initialData[0].data = &value;
	ctcd.initialData[0].pitch = 4;
	ctcd.initialData[1].data = &value;
	ctcd.initialData[1].pitch = 4;
	ctcd.initialData[2].data = &value;
	ctcd.initialData[2].pitch = 4;
	ctcd.initialData[3].data = &value;
	ctcd.initialData[3].pitch = 4;
	ctcd.initialData[4].data = &value;
	ctcd.initialData[4].pitch = 4;
	ctcd.initialData[5].data = &value;
	ctcd.initialData[0].pitch = 4;
	return renderSystem->createCubeTexture(ctcd, T_FILE_LINE_W);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRendererShared", WorldRendererShared, IWorldRenderer)

bool WorldRendererShared::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;

	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_shadowsQuality = desc.quality.shadows;
	m_sharedDepthStencil = desc.sharedDepthStencil;
	
	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Create default value textures.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);
	m_blackCubeTexture = createCubeTexture(renderSystem, 0x00000000);

	// Lights struct buffer.
	m_lightSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		LightClusterPass::c_maxLightCount,
		sizeof(LightShaderData),
		true
	);
	if (!m_lightSBuffer)
		return false;

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	m_shadowAtlasPacker = new Packer(
		shadowSettings.resolution,
		shadowSettings.resolution
	);

	// Create irradiance grid.
	if (desc.irradianceGrid != nullptr)
		m_irradianceGrid = resource::Proxy< IrradianceGrid >(const_cast< IrradianceGrid* >(desc.irradianceGrid));
	else if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Create "clear depth" shader.
	if (!resourceManager->bind(c_clearDepthShader, m_clearDepthShader))
		return false;

	// Determine slice distances.
	for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
	{
		const float ii = float(i) / shadowSettings.cascadingSlices;
		const float log = powf(ii, shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
	}
	m_slicePositions[shadowSettings.cascadingSlices] = shadowSettings.farZ;

	// Create shared passes.
	m_lightClusterPass = new LightClusterPass(m_settings);
	if (!m_lightClusterPass->create(renderSystem))
		return false;

	m_gbufferPass = new GBufferPass(m_settings, m_entityRenderers, m_sharedDepthStencil);

	m_velocityPass = new VelocityPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_velocityPass->create(resourceManager, renderSystem, desc))
		return false;

	m_ambientOcclusionPass = new AmbientOcclusionPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_ambientOcclusionPass->create(resourceManager, renderSystem, desc))
		return false;

	m_reflectionsPass = new ReflectionsPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_reflectionsPass->create(resourceManager, renderSystem, desc))
		return false;

	m_postProcessPass = new PostProcessPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_postProcessPass->create(resourceManager, renderSystem, desc))
		return false;

	return true;
}

void WorldRendererShared::destroy()
{
	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);
	safeDestroy(m_lightSBuffer);

	m_postProcessPass = nullptr;
	m_reflectionsPass = nullptr;
	m_ambientOcclusionPass = nullptr;
	m_velocityPass = nullptr;
	m_gbufferPass = nullptr;

	safeDestroy(m_lightClusterPass);

	m_irradianceGrid.clear();
}

void WorldRendererShared::gather(Entity* rootEntity)
{
	T_PROFILER_SCOPE(L"WorldRendererShared::gather");
	StaticVector< const LightComponent*, LightClusterPass::c_maxLightCount > lights;

	m_gatheredView.renderables.resize(0);
	m_gatheredView.lights.resize(0);
	m_gatheredView.probes.resize(0);
	m_gatheredView.fogs.resize(0);

	WorldGatherContext(m_entityRenderers, rootEntity, [&](IEntityRenderer* entityRenderer, Object* renderable) {

		// Filter out components used to setup frame's lighting etc.
		if (auto lightComponent = dynamic_type_cast< const LightComponent* >(renderable))
		{
			if (!lights.full())
				lights.push_back(lightComponent);
		}
		else if (auto probeComponent = dynamic_type_cast< const ProbeComponent* >(renderable))
			m_gatheredView.probes.push_back(probeComponent);
		else if (auto volumetricFogComponent = dynamic_type_cast< const VolumetricFogComponent* >(renderable))
			m_gatheredView.fogs.push_back(volumetricFogComponent);

		m_gatheredView.renderables.push_back({ entityRenderer, renderable });

	}).gather(rootEntity);

	// Arrange lights.
	{
		const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

		m_gatheredView.lights.resize(4);
		m_gatheredView.lights[0] = nullptr;
		m_gatheredView.lights[1] = nullptr;
		m_gatheredView.lights[2] = nullptr;
		m_gatheredView.lights[3] = nullptr;

		// Find cascade shadow light; must be first.
		if (shadowsEnable)
		{
			for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
			{
				auto& light = lights[i];
				if (
					light->getCastShadow() &&
					light->getLightType() == LightType::Directional
				)
				{
					m_gatheredView.lights[0] = light;
					break;
				}
			}
		}

		// Add all other lights.
		for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
		{
			auto& light = lights[i];
			if (light != m_gatheredView.lights[0])
				m_gatheredView.lights.push_back(light);
		}
	}
}

void WorldRendererShared::setupLightPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t& outShadowMapAtlasTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupLightPass");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);

	LightShaderData* lightShaderData = (LightShaderData*)m_lightSBuffer->lock();
	if (!lightShaderData)
		return;

	// Reset this frame's atlas packer.
	auto shadowAtlasPacker = m_shadowAtlasPacker;
	shadowAtlasPacker->reset();

	const Matrix44 lastView = worldRenderView.getLastView();
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
		outShadowMapAtlasTargetSetId = renderGraph.addPersistentTargetSet(
			L"Shadow map atlas",
			render::getParameterHandle(L"ShadowMap"),
			false,
			rgtd
		);

		// Add shadow map render passes.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow map");
		rp->setOutput(outShadowMapAtlasTargetSetId, render::TfNone, render::TfDepth);

		if (m_gatheredView.lights[0] != nullptr)
		{
			const auto& light = m_gatheredView.lights[0];
			const Transform lightTransform = light->getTransform();
			const Vector4 lightPosition = lightTransform.translation().xyz1();
			const Vector4 lightDirection = lightTransform.axisY().xyz0();

			// Update one slice per frame.
			//const int32_t updateSlices[] = { 0, (m_count % (shadowSettings.cascadingSlices - 1)) + 1 };
			for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
			{
				const int32_t slice = i; //updateSlices[i];
				const Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				const Scalar zf(min(m_slicePositions[slice + 1], shadowSettings.farZ));

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

				// Check if this slice is still inside
				// the expanded slice frustum.
				if (slice != 0)
				{
					const Matrix44 viewDelta = view * lastView.inverse();
					const int32_t force = (m_count % (shadowSettings.cascadingSlices - 1)) + 1;
					if (slice != force && m_shadowSlices[i].inside(viewDelta, sliceViewFrustum) == Frustum::IrInside)
						continue;
					sliceViewFrustum.scale(1.0_simd);
				}

				m_shadowSlices[i] = sliceViewFrustum;

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

				m_shadowLightView[slice] = shadowLightProjection * shadowLightView;

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

						// Clear cascade shadow map.
						{
							const render::Shader::Permutation perm;
							m_screenRenderer->draw(renderContext, m_clearDepthShader, perm, nullptr);
						}

						for (auto r : m_gatheredView.renderables)
							r.renderer->build(wc, shadowRenderView, shadowPass, r.renderable);
	
						for (auto entityRenderer : m_entityRenderers->get())
							entityRenderer->build(wc, shadowRenderView, shadowPass);
					}
				);
			}

			// Expose slice data to shaders.
			auto* lsd = lightShaderData;
			for (int32_t slice = 0; slice < shadowSettings.cascadingSlices; ++slice)
			{
				const Matrix44 viewToLightSpace = m_shadowLightView[slice] * viewInverse;

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
			const int32_t atlasSize = 256 >> std::min(denom, 8);
					
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

}
