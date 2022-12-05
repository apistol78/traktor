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
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Shared/WorldRendererShared.h"
#include "World/Shared/WorldRenderPassShared.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::ImageGraph > c_velocityPrime(L"{CB34E98B-55C9-E447-BD59-5A1D91DCA88E}");
const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{416745F9-93C7-8D45-AE28-F2823DEE636A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{45F9CD9F-C700-9942-BB36-443629C88748}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{302E57C8-711D-094F-A764-75F76553E81B}");
const resource::Id< render::ImageGraph > c_antiAliasLow(L"{71D385F1-8364-C849-927F-5F1249F5DF92}");
const resource::Id< render::ImageGraph > c_antiAliasMedium(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}");
const resource::Id< render::ImageGraph > c_antiAliasHigh(L"{C0316981-FA73-A34E-8135-1F596425688F}");
const resource::Id< render::ImageGraph > c_antiAliasUltra(L"{88E329C8-A2F3-7443-B73E-4E85C6ECACBE}");
const resource::Id< render::ImageGraph > c_gammaCorrection(L"{B1E8367D-91DD-D648-A44F-B86492169771}");
const resource::Id< render::ImageGraph > c_toneMapFixed(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}");
const resource::Id< render::ImageGraph > c_toneMapAdaptive(L"{BE19DE90-E010-A74D-AA3B-87FAC2A56946}");
const resource::Id< render::ImageGraph > c_screenReflections(L"{2F8EC56A-FD46-DF42-94B5-9DD676B8DD8A}");

resource::Id< render::ImageGraph > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case Quality::Disabled:
		return resource::Id< render::ImageGraph >();
	case Quality::Low:
		return c_ambientOcclusionLow;
	case Quality::Medium:
		return c_ambientOcclusionMedium;
	case Quality::High:
		return c_ambientOcclusionHigh;
	case Quality::Ultra:
		return c_ambientOcclusionUltra;
	}
}

resource::Id< render::ImageGraph > getAntiAliasId(Quality quality)
{
	switch (quality)
	{
	default:
	case Quality::Disabled:
		return resource::Id< render::ImageGraph >();
	case Quality::Low:
		return c_antiAliasLow;
	case Quality::Medium:
		return c_antiAliasMedium;
	case Quality::High:
		return c_antiAliasHigh;
	case Quality::Ultra:
		return c_antiAliasUltra;
	}
}

resource::Id< render::ImageGraph > getToneMapId(WorldRenderSettings::ExposureMode exposureMode)
{
	switch (exposureMode)
	{
	default:
	case WorldRenderSettings::EmFixed:
		return c_toneMapFixed;
	case WorldRenderSettings::EmAdaptive:
		return c_toneMapAdaptive;
	}
}

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

Vector2 jitter(int32_t count)
{
	const Vector2 kernelSize(0.5f, 0.5f);
	return Vector2(
		(float)((count / 2) & 1) * kernelSize.x - kernelSize.x / 2.0f,
		(float)(      count & 1) * kernelSize.y - kernelSize.y / 2.0f
	);
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
	m_toneMapQuality = desc.quality.toneMap;
	m_motionBlurQuality = desc.quality.motionBlur;
	m_shadowsQuality = desc.quality.shadows;
	m_reflectionsQuality = desc.quality.reflections;
	m_ambientOcclusionQuality = desc.quality.ambientOcclusion;
	m_antiAliasQuality = desc.quality.antiAlias;
	m_gamma = desc.gamma;
	m_sharedDepthStencil = desc.sharedDepthStencil;

	// Create velocity prime processing; priming is also used by TAA.
	if (
		m_motionBlurQuality > Quality::Disabled ||
		m_antiAliasQuality >= Quality::Ultra
	)
	{
		if (!resourceManager->bind(c_velocityPrime, m_velocityPrime))
		{
			log::warning << L"Unable to create velocity prime process; disabled." << Endl;
			m_motionBlurQuality = Quality::Disabled;
		}
	}

	// Create ambient occlusion processing.
	if (m_ambientOcclusionQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > ambientOcclusion = getAmbientOcclusionId(m_ambientOcclusionQuality);
		if (!resourceManager->bind(ambientOcclusion, m_ambientOcclusion))
			log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
	}

	// Create antialias processing.
	if (m_antiAliasQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > antiAlias = getAntiAliasId(m_antiAliasQuality);
		if (!resourceManager->bind(antiAlias, m_antiAlias))
			log::warning << L"Unable to create antialias process; AA disabled." << Endl;
	}

	// Create "visual" post processing filter.
	if (desc.quality.imageProcess > Quality::Disabled)
	{
		const auto& visualImageGraph = desc.worldRenderSettings->imageProcess[(int32_t)desc.quality.imageProcess];
		if (!visualImageGraph.isNull())
		{
			if (!resourceManager->bind(visualImageGraph, m_visual))
				log::warning << L"Unable to create visual post processing; post processing disabled." << Endl;
		}
	}

	// Create gamma correction processing.
	if (std::abs(m_gamma - 1.0f) > FUZZY_EPSILON)
	{
		if (!resourceManager->bind(c_gammaCorrection, m_gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
	}

	// Create tone map processing.
	if (m_toneMapQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > toneMap = getToneMapId(m_settings.exposureMode);
		if (!resourceManager->bind(toneMap, m_toneMap))
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = Quality::Disabled;
		}		
	}

	// Create screen reflections processing.
	if (m_reflectionsQuality >= Quality::Disabled)
	{
		if (!resourceManager->bind(c_screenReflections, m_screenReflections))
		{
			log::warning << L"Unable to create screen space reflections process." << Endl;
			m_reflectionsQuality = Quality::Disabled;
		}
	}

	m_imageGraphContext = new render::ImageGraphContext();
	
	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Lights struct buffer.
	m_lightSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		c_maxLightCount,
		sizeof(LightShaderData),
		true
	);
	if (!m_lightSBuffer)
		return false;

	// Tile light index array buffer.
	m_lightIndexSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ * MaxLightsPerCluster,
		sizeof(LightIndexShaderData),
		true
	);
	if (!m_lightIndexSBuffer)
		return false;

	// Tile cluster buffer.
	m_tileSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ,
		sizeof(TileShaderData),
		true
	);
	if (!m_tileSBuffer)
		return false;

	// Create default value textures.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);

	return true;
}

void WorldRendererShared::destroy()
{
	safeDestroy(m_lightSBuffer);
	safeDestroy(m_lightIndexSBuffer);
	safeDestroy(m_tileSBuffer);
	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);
}

render::ImageGraphContext* WorldRendererShared::getImageGraphContext() const
{
	return m_imageGraphContext;
}

void WorldRendererShared::setupTileDataPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupTileDataPass");
#if defined(T_WORLD_USE_TILE_JOB)
	// Enqueue light clustering as a job, is synchronized in before rendering.
	m_tileJob = JobManager::getInstance().add([=]() {
#endif
		const auto& viewFrustum = worldRenderView.getViewFrustum();

		TileShaderData* tileShaderData = (TileShaderData*)m_tileSBuffer->lock();
		std::memset(tileShaderData, 0, ClusterDimXY * ClusterDimXY * ClusterDimZ * sizeof(TileShaderData));

		LightIndexShaderData* lightIndexShaderData = (LightIndexShaderData*)m_lightIndexSBuffer->lock();
		std::memset(lightIndexShaderData, 0, ClusterDimXY * ClusterDimXY * ClusterDimZ * MaxLightsPerCluster * sizeof(LightIndexShaderData));

		StaticVector< Vector4, c_maxLightCount > lightPositions;
		StaticVector< int32_t, c_maxLightCount > sliceLights;

		// Calculate positions of lights in view space.
		for (const auto& light : m_lights)
		{
			Vector4 lightPosition = light->getTransform().translation().xyz1();
			lightPositions.push_back(worldRenderView.getView() * lightPosition);
		}

		// Update tile data.
		const Scalar dx(1.0f / ClusterDimXY);
		const Scalar dy(1.0f / ClusterDimXY);
		const Scalar dz(1.0f / ClusterDimZ);

		const Vector4& tl = viewFrustum.corners[0];
		const Vector4& tr = viewFrustum.corners[1];
		const Vector4& bl = viewFrustum.corners[3];

		const Vector4 vx = tr - tl;
		const Vector4 vy = bl - tl;

		const Scalar vnz = viewFrustum.getNearZ();
		const Scalar vfz = viewFrustum.getFarZ();

		// Calculate XY tile frustums.
		Frustum tileFrustums[ClusterDimXY * ClusterDimXY];
		for (int32_t y = 0; y < ClusterDimXY; ++y)
		{
			const Scalar fy = Scalar((float)y) * dy;
			for (int32_t x = 0; x < ClusterDimXY; ++x)
			{
				const Scalar fx = Scalar((float)x) * dx;
				
				const Vector4 a = tl + vx * fx + vy * fy;
				const Vector4 b = tl + vx * (fx + dx) + vy * fy;
				const Vector4 c = tl + vx * (fx + dx) + vy * (fy + dy);
				const Vector4 d = tl + vx * fx + vy * (fy + dy);
				
				auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
				tileFrustum.planes[Frustum::PsLeft] = Plane(Vector4::zero(), d, a);
				tileFrustum.planes[Frustum::PsRight] = Plane(Vector4::zero(), b, c);
				tileFrustum.planes[Frustum::PsBottom] = Plane(Vector4::zero(), c, d);
				tileFrustum.planes[Frustum::PsTop] = Plane(Vector4::zero(), a, b);
			}
		}

		// Group lights per cluster.
		uint32_t lightOffset = 0;
		for (int32_t z = 0; z < ClusterDimZ; ++z)
		{
			const Scalar snz = vnz * power(vfz / vnz, Scalar(z) / Scalar(ClusterDimZ));
			const Scalar sfz = vnz * power(vfz / vnz, Scalar(z + 1) / Scalar(ClusterDimZ));

			// Gather all lights intersecting slice.
			sliceLights.clear();
			for (uint32_t i = 0; i < m_lights.size(); ++i)
			{
				const auto light = m_lights[i];
				if (light->getLightType() == LightType::Directional)
				{
					sliceLights.push_back(i);
				}
				else if (light->getLightType() == LightType::Point)
				{
					const Scalar lr = light->getRange();				
					const Scalar lz = lightPositions[i].z();
					if (lz + lr >= snz && lz - lr <= sfz)
						sliceLights.push_back(i);
				}
				else if (light->getLightType() == LightType::Spot)
				{
					const Scalar lr = light->getRange();				
					const Scalar lz = lightPositions[i].z();
					if (lz + lr >= snz && lz - lr <= sfz)
					{
						//Frustum spotFrustum;
						//spotFrustum.buildPerspective(light.radius, 1.0f, 0.0f, light.range);

						//Vector4 p[4];
						//p[0] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[4].xyz0();
						//p[1] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[5].xyz0();
						//p[2] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[6].xyz0();
						//p[3] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[7].xyz0();

						//Range< Scalar > bb;
						//bb.min = lz;
						//bb.max = lz;
						//for (int i = 0; i < 4; ++i)
						//{
						//	bb.min = min(bb.min, p[i].z());
						//	bb.max = max(bb.max, p[i].z());
						//}
						//if (Range< Scalar >::intersection(bb, Range< Scalar >(snz, sfz)).delta() > 0.0_simd)
							sliceLights.push_back(i);
					}
				}
			}

			if (sliceLights.empty())
				continue;

			for (int32_t y = 0; y < ClusterDimXY; ++y)
			{
				for (int32_t x = 0; x < ClusterDimXY; ++x)
				{
					auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
					tileFrustum.planes[Frustum::PsNear] = Plane(Vector4(0.0f, 0.0f, 1.0f), snz);
					tileFrustum.planes[Frustum::PsFar] = Plane(Vector4(0.0f, 0.0f, -1.0f), -sfz);

					int32_t count = 0;
					for (uint32_t i = 0; i < sliceLights.size(); ++i)
					{
						const int32_t lightIndex = sliceLights[i];
						const auto light = m_lights[lightIndex];
						if (light->getLightType() == LightType::Directional)
						{
							lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
							++count;
						}
						else if (light->getLightType() == LightType::Point)
						{
							if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::IrOutside)
							{
								lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
								++count;
							}
						}
						else if (light->getLightType() == LightType::Spot)
						{
							// \fixme Implement frustum to frustum culling.
							if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::IrOutside)
							{
								lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
								++count;
							}
						}
						if (count >= MaxLightsPerCluster)
							break;
					}

					const uint32_t tileOffset = x + y * ClusterDimXY + z * ClusterDimXY * ClusterDimXY;
					tileShaderData[tileOffset].lightOffsetAndCount[0] = (int32_t)lightOffset;
					tileShaderData[tileOffset].lightOffsetAndCount[1] = count;

					lightOffset += count;
				}
			}
		}

		m_lightIndexSBuffer->unlock();
		m_tileSBuffer->unlock();
#if defined(T_WORLD_USE_TILE_JOB)
	});
#endif
}

render::handle_t WorldRendererShared::setupGBufferPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupGBufferPass");
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 2;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR16G16F;	// Depth (R), Roughness (G)
	rgtd.targets[1].colorFormat = render::TfR11G11B10F;	// Normals (RGB)
	auto gbufferTargetSetId = renderGraph.addTransientTargetSet(L"GBuffer", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");
	
	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(clearZ, 1.0f, 0.0f, 0.0f);
	clear.colors[1] = Color4f(0.5f, 0.5f, 0.0f, 0.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;
	rp->setOutput(gbufferTargetSetId, clear, render::TfNone, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			const WorldRenderPassShared gbufferPass(
				s_techniqueForwardGBufferWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfFirst
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, gbufferPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, gbufferPass);

			renderContext->merge(render::RpAll);		
		}
	);

	renderGraph.addPass(rp);
	return gbufferTargetSetId;
}

render::handle_t WorldRendererShared::setupVelocityPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupVelocityPass");

	// Add Velocity target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR32G32F;
	auto velocityTargetSetId = renderGraph.addTransientTargetSet(L"Velocity", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add Velocity render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Velocity");
	
	if (m_velocityPrime)
	{
		render::ImageGraphView view;

		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getLastView() * worldRenderView.getView().inverse();
		view.projection = worldRenderView.getProjection();
		view.deltaTime = worldRenderView.getDeltaTime();

		m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);

		m_velocityPrime->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			*m_imageGraphContext,
			view
		);
	}

	rp->setOutput(velocityTargetSetId, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			const WorldRenderPassShared velocityPass(
				s_techniqueVelocityWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone
			);

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, velocityPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, velocityPass);

			renderContext->merge(render::RpAll);
		}
	);

	renderGraph.addPass(rp);
	return velocityTargetSetId;
}

render::handle_t WorldRendererShared::setupAmbientOcclusionPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	render::ImageGraphView view;

	if (m_ambientOcclusion == nullptr)
		return 0;

	T_PROFILER_SCOPE(L"WorldRendererShared setupAmbientOcclusionPass");

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	auto ambientOcclusionTargetSetId = renderGraph.addTransientTargetSet(L"Ambient occlusion", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add ambient occlusion render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");
	m_ambientOcclusion->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		*m_imageGraphContext,
		view
	);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(ambientOcclusionTargetSetId, clear, render::TfNone, render::TfColor);

	renderGraph.addPass(rp);
	return ambientOcclusionTargetSetId;
}

render::handle_t WorldRendererShared::setupReflectionsPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t visualReadTargetSetId
) const
{
	if (m_reflectionsQuality == Quality::Disabled)
		return 0;

	T_PROFILER_SCOPE(L"WorldRendererShared setupReflectionsPass");

	// Add reflections target.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.ignoreStencil = true;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;

	switch (m_reflectionsQuality)
	{
	default:
	case Quality::Low:
		rgtd.referenceWidthDenom = 2;	// 50%
		rgtd.referenceHeightDenom = 2;
		break;

	case Quality::Medium:
		rgtd.referenceWidthMul = 2;		// 67%
		rgtd.referenceWidthDenom = 3;
		rgtd.referenceHeightMul = 2;
		rgtd.referenceHeightDenom = 3;
		break;

	case Quality::High:
		rgtd.referenceWidthMul = 4;		// 80%
		rgtd.referenceWidthDenom = 5;
		rgtd.referenceHeightMul = 4;
		rgtd.referenceHeightDenom = 5;
		break;

	case Quality::Ultra:
		rgtd.referenceWidthDenom = 1;	// 100%
		rgtd.referenceHeightDenom = 1;
		break;
	}

	auto reflectionsTargetSetId = renderGraph.addTransientTargetSet(L"Reflections", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add reflections render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Reflections");

	rp->addInput(gbufferTargetSetId);
	rp->addInput(visualReadTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(reflectionsTargetSetId, clear, render::TfNone, render::TfColor);
	
	// rp->addBuild(
	// 	[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
	// 	{
	// 		WorldBuildContext wc(
	// 			m_entityRenderers,
	// 			rootEntity,
	// 			renderContext
	// 		);

	// 		auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);

	// 		auto sharedParams = renderContext->alloc< render::ProgramParameters >();
	// 		sharedParams->beginParameters(renderContext);
	// 		sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	// 		sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
	// 		sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
	// 		sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	// 		sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
	// 		sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
	// 		sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
	// 		sharedParams->endParameters(renderContext);

	// 		WorldRenderPassShared reflectionsPass(
	// 			s_techniqueReflectionWrite,
	// 			sharedParams,
	// 			worldRenderView,
	// 			IWorldRenderPass::PfNone
	// 		);

	// 		T_ASSERT(!renderContext->havePendingDraws());

	// 		for (auto gathered : m_gathered)
	// 			gathered.entityRenderer->build(wc, worldRenderView, reflectionsPass, gathered.renderable);
	
	// 		for (auto entityRenderer : m_entityRenderers->get())
	// 			entityRenderer->build(wc, worldRenderView, reflectionsPass);

	// 		renderContext->merge(render::RpAll);
	// 	}
	// );
	
	 // Render screenspace reflections.
	render::ImageGraphView view;
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = worldRenderView.getDeltaTime();

	m_imageGraphContext->associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputRoughness, gbufferTargetSetId, 1);

	m_screenReflections->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		*m_imageGraphContext,
		view
	);

	renderGraph.addPass(rp);
	return reflectionsTargetSetId;
}

void WorldRendererShared::setupProcessPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t velocityTargetSetId,
	render::handle_t visualWriteTargetSetId,
	render::handle_t visualReadTargetSetId
) const
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupProcessPass");
	render::ImageGraphView view;

	view.viewFrustum = worldRenderView.getViewFrustum();
	view.viewToLight = Matrix44::identity();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = worldRenderView.getDeltaTime();
	view.time = worldRenderView.getTime();

	m_imageGraphContext->associateTextureTargetSet(s_handleInputColor, visualWriteTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputVelocity, velocityTargetSetId, 0);

	// Expose gamma and exposure.
	m_imageGraphContext->setFloatParameter(s_handleGamma, m_gamma);
	m_imageGraphContext->setFloatParameter(s_handleGammaInverse, 1.0f / m_gamma);
	m_imageGraphContext->setFloatParameter(s_handleExposure, std::pow(2.0f, m_settings.exposure));

	// Expose jitter; in texture space.
	const Vector2 rc = jitter(m_count) / worldRenderView.getViewSize();
	const Vector2 rp = jitter(m_count - 1) / worldRenderView.getViewSize();
	m_imageGraphContext->setVectorParameter(s_handleJitter, Vector4(rp.x, -rp.y, rc.x, -rc.y));

	StaticVector< render::ImageGraph*, 4 > processes;
	if (m_toneMap)
		processes.push_back(m_toneMap);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);
	if (m_visual)
		processes.push_back(m_visual);
	if (m_gammaCorrection)
		processes.push_back(m_gammaCorrection);

	render::handle_t intermediateTargetSetId = 0;
	for (size_t i = 0; i < processes.size(); ++i)
	{
		auto process = processes[i];
		const bool next = (bool)((i + 1) < processes.size());

		Ref< render::RenderPass > rp = new render::RenderPass(L"Process");

		if (next)
		{
			render::RenderGraphTargetSetDesc rgtd;
			rgtd.count = 1;
			rgtd.createDepthStencil = false;
			rgtd.usingPrimaryDepthStencil = false;
			rgtd.referenceWidthDenom = 1;
			rgtd.referenceHeightDenom = 1;
			rgtd.targets[0].colorFormat = render::TfR11G11B10F;
			intermediateTargetSetId = renderGraph.addTransientTargetSet(L"Process intermediate", rgtd, nullptr, outputTargetSetId);

			rp->setOutput(intermediateTargetSetId, render::TfColor, render::TfColor);
		}
		else
		{
			render::Clear cl;
			cl.mask = render::CfColor;
			cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			rp->setOutput(outputTargetSetId, cl, render::TfDepth, render::TfColor | render::TfDepth);
		}

		process->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			*m_imageGraphContext,
			view
		);

		if (next)
			m_imageGraphContext->associateTextureTargetSet(s_handleInputColor, intermediateTargetSetId, 0);

		renderGraph.addPass(rp);
	}
}

}
