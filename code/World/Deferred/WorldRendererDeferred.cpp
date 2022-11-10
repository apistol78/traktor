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
#include "World/WorldGatherContext.h"
#include "World/WorldHandles.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/LightComponent.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Deferred/WorldRenderPassDeferred.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor::world
{
	namespace
	{

const int32_t c_maxLightCount = 1024;

const render::Handle s_handleVisualTargetSet[] =
{
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

const resource::Id< render::Shader > c_lightShader(L"{707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}");
const resource::Id< render::Shader > c_fogShader(L"{9453D74C-76C4-8748-9A5B-9E3D6D4F9406}");

const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{416745F9-93C7-8D45-AE28-F2823DEE636A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{45F9CD9F-C700-9942-BB36-443629C88748}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{302E57C8-711D-094F-A764-75F76553E81B}");
const resource::Id< render::ImageGraph > c_antiAliasLow(L"{71D385F1-8364-C849-927F-5F1249F5DF92}");
const resource::Id< render::ImageGraph > c_antiAliasMedium(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}");
const resource::Id< render::ImageGraph > c_antiAliasHigh(L"{C0316981-FA73-A34E-8135-1F596425688F}");
const resource::Id< render::ImageGraph > c_antiAliasUltra(L"{88E329C8-A2F3-7443-B73E-4E85C6ECACBE}");
const resource::Id< render::ImageGraph > c_gammaCorrection(L"{B1E8367D-91DD-D648-A44F-B86492169771}");
const resource::Id< render::ImageGraph > c_motionBlurPrime(L"{CB34E98B-55C9-E447-BD59-5A1D91DCA88E}");
const resource::Id< render::ImageGraph > c_motionBlurLow(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}");
const resource::Id< render::ImageGraph > c_motionBlurMedium(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}");
const resource::Id< render::ImageGraph > c_motionBlurHigh(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}");
const resource::Id< render::ImageGraph > c_motionBlurUltra(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}");
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

resource::Id< render::ImageGraph > getMotionBlurId(Quality quality)
{
	switch (quality)
	{
	default:
	case Quality::Disabled:
		return resource::Id< render::ImageGraph >();
	case Quality::Low:
		return c_motionBlurLow;
	case Quality::Medium:
		return c_motionBlurMedium;
	case Quality::High:
		return c_motionBlurHigh;
	case Quality::Ultra:
		return c_motionBlurUltra;
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

#pragma pack(1)
struct LightShaderData
{
	float typeRangeRadius[4];
	float position[4];
	float direction[4];
	float color[4];
	float viewToLight0[4];
	float viewToLight1[4];
	float viewToLight2[4];
	float viewToLight3[4];
	float atlasTransform[4];
};
#pragma pack()

#pragma pack(1)
struct TileShaderData
{
	uint16_t lights[4];			// 8 :  0 -  8
	uint8_t lightCount[4];		// 4 :  8 - 12
	uint8_t pad[4];				// 4 : 12 - 16
};
#pragma pack()

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, IWorldRenderer)

bool WorldRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.quality.toneMap;
	m_motionBlurQuality = desc.quality.motionBlur;
	m_shadowSettings = m_settings.shadowSettings[(int32_t)desc.quality.shadows];
	m_shadowsQuality = desc.quality.shadows;
	m_reflectionsQuality = desc.quality.reflections;
	m_ambientOcclusionQuality = desc.quality.ambientOcclusion;
	m_antiAliasQuality = desc.quality.antiAlias;
	m_gamma = desc.gamma;
	m_sharedDepthStencil = desc.sharedDepthStencil;

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
	if (std::abs(desc.gamma - 1.0f) > FUZZY_EPSILON)
	{
		if (!resourceManager->bind(c_gammaCorrection, m_gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
	}

	// Create motion blur prime processing; priming is also used by TAA.
	if (
		m_motionBlurQuality > Quality::Disabled ||
		m_antiAliasQuality >= Quality::Ultra
	)
	{
		if (!resourceManager->bind(c_motionBlurPrime, m_velocityPrime))
		{
			log::warning << L"Unable to create motion blur prime process; motion blur disabled." << Endl;
			m_motionBlurQuality = Quality::Disabled;
		}
	}

	// Create motion blur final processing.
	if (m_motionBlurQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > motionBlur = getMotionBlurId(desc.quality.motionBlur);
		if (!resourceManager->bind(motionBlur, m_motionBlur))
		{
			log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
			m_motionBlurQuality = Quality::Disabled;
		}
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
	if (m_reflectionsQuality >= Quality::High)
	{
		if (!resourceManager->bind(c_screenReflections, m_screenReflections))
		{
			log::warning << L"Unable to create screen space reflections process." << Endl;
			m_reflectionsQuality = Quality::Disabled;
		}
	}

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

	// Allocate light lists.
	m_lightSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		c_maxLightCount,
		sizeof(LightShaderData),
		true
	);
	if (!m_lightSBuffer)
		return false;

	m_lightSBufferMemory = m_lightSBuffer->lock();
	if (!m_lightSBufferMemory)
		return false;

	m_tileSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ,
		sizeof(TileShaderData),
		true
	);
	if (!m_tileSBuffer)
		return false;

	m_tileSBufferMemory = m_tileSBuffer->lock();
	if (!m_tileSBufferMemory)
		return false;

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / m_shadowSettings.cascadingSlices;
		float log = powf(ii, m_shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, m_shadowSettings.farZ, log);
	}
	m_slicePositions[m_shadowSettings.cascadingSlices] = m_shadowSettings.farZ;

	m_entityRenderers = desc.entityRenderers;

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Misc resources.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);

	return true;
}

void WorldRendererDeferred::destroy()
{
	safeDestroy(m_lightSBuffer);
	safeDestroy(m_tileSBuffer);
	safeDestroy(m_shadowMapCascadeTargetSet);
	safeDestroy(m_shadowMapAtlasTargetSet);
	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);

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
	if (m_antiAliasQuality >= Quality::Ultra)
	{
		Vector2 r = (jitter(m_count) * 2.0f) / worldRenderView.getViewSize();
		Matrix44 proj = immutableWorldRenderView.getProjection();
		proj = translate(r.x, r.y, 0.0f) * proj;
		worldRenderView.setProjection(proj);
	}

	// Gather active renderables for this frame.
	{
		T_PROFILER_SCOPE(L"WorldRendererDeferred gather");

		m_lights.resize(0);
		m_gathered.resize(0);

		WorldGatherContext(m_entityRenderers, rootEntity, [&](IEntityRenderer* entityRenderer, Object* renderable) {

			// Gather lights separately as we need them for shadows and lighting.
			if (auto lightComponent = dynamic_type_cast< const LightComponent* >(renderable))
				m_lights.push_back(lightComponent);

			m_gathered.push_back({ entityRenderer, renderable });

		}).gather(const_cast< Entity* >(rootEntity));

		if (m_lights.size() > c_maxLightCount)
			m_lights.resize(c_maxLightCount);
	}

	// Begun writing light shader data; written both in setup and build.
	LightShaderData* lightShaderData = (LightShaderData*)m_lightSBufferMemory;
	TileShaderData* tileShaderData = (TileShaderData*)m_tileSBufferMemory;

	// Write all lights to sbuffer; without shadow map information.
	const Matrix44& view = worldRenderView.getView();
	for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
	{
		const auto light = m_lights[i];
		auto* lsd = &lightShaderData[i];

		lsd->typeRangeRadius[0] = (float)light->getLightType();
		lsd->typeRangeRadius[1] = light->getRange();
		lsd->typeRangeRadius[2] = std::cos((light->getRadius() - deg2rad(8.0f)) / 2.0f);
		lsd->typeRangeRadius[3] = std::cos(light->getRadius() / 2.0f);

		Transform lightTransform = Transform(view) * light->getTransform();
		lightTransform.translation().xyz1().storeUnaligned(lsd->position);
		lightTransform.axisY().xyz0().storeUnaligned(lsd->direction);
		light->getColor().storeUnaligned(lsd->color);

		Vector4::zero().storeUnaligned(lsd->viewToLight0);
		Vector4::zero().storeUnaligned(lsd->viewToLight1);
		Vector4::zero().storeUnaligned(lsd->viewToLight2);
		Vector4::zero().storeUnaligned(lsd->viewToLight3);
	}

	// Find directional light for cascade shadow map.
	int32_t lightCascadeIndex = -1;
	if (m_shadowsQuality != Quality::Disabled)
	{
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto light = m_lights[i];
			if (light->getCastShadow() && light->getLightType() == LightType::LtDirectional)
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
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto light = m_lights[i];
			if (light->getCastShadow() && light->getLightType() == LightType::LtSpot)
				lightAtlasIndices.push_back(i);
		}
	}

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererDeferred setup extra passes");
		WorldSetupContext context(m_entityRenderers, rootEntity, renderGraph);

		for (auto gathered : m_gathered)
			gathered.entityRenderer->setup(context, worldRenderView, gathered.renderable);
	
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
	auto gbufferTargetSetId = setupGBufferPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId
	);

	auto velocityTargetSetId = setupVelocityPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId
	);

	auto ambientOcclusionTargetSetId = setupAmbientOcclusionPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId
	);

	auto shadowMapCascadeTargetSetId = setupCascadeShadowMapPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		lightCascadeIndex,
		lightShaderData
	);

	auto shadowMapAtlasTargetSetId = setupAtlasShadowMapPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		lightAtlasIndices,
		lightShaderData
	);

	setupTileDataPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		tileShaderData
	);

	auto shadowMaskTargetSetId = setupShadowMaskPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		shadowMapCascadeTargetSetId,
		lightCascadeIndex
	);

	auto reflectionsTargetSetId = setupReflectionsPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		visualReadTargetSetId
	);

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

	setupProcessPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		velocityTargetSetId,
		visualWriteTargetSetId,
		visualReadTargetSetId
	);

	m_count++;
}

render::ImageGraphContext* WorldRendererDeferred::getImageGraphContext() const
{
	return nullptr;
}

render::handle_t WorldRendererDeferred::setupGBufferPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"World setup gbuffer");
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 4;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR32F;		// Depth (R)
	rgtd.targets[1].colorFormat = render::TfR16G16F;	// Normals (RG)
	rgtd.targets[2].colorFormat = render::TfR11G11B10F;	// Metalness (R), Roughness (G), Specular (B)
	rgtd.targets[3].colorFormat = render::TfR11G11B10F;	// Surface color (RGB)
	auto gbufferTargetSetId = renderGraph.addTransientTargetSet(L"GBuffer", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");

	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);	// depth
	clear.colors[1] = Color4f(0.0f, 0.0f, 1.0f, 0.0f);	// normal
	clear.colors[2] = Color4f(0.0f, 1.0f, 0.0f, 1.0f);	// misc
	clear.colors[3] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);	// surface
	clear.depth = 1.0f;	
	clear.stencil = 0;
	rp->setOutput(gbufferTargetSetId, clear, render::TfNone, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->endParameters(renderContext);

			WorldRenderPassDeferred gbufferPass(
				s_techniqueDeferredGBufferWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfFirst,
				false
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

render::handle_t WorldRendererDeferred::setupVelocityPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	//if (m_motionBlurQuality == Quality::Disabled)
	//	return 0;

	T_PROFILER_SCOPE(L"World setup velocity");

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
		render::ImageGraphContext context;
		render::ImageGraphView view;

		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getLastView() * worldRenderView.getView().inverse();
		view.projection = worldRenderView.getProjection();
		view.deltaTime = worldRenderView.getDeltaTime();

		context.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);

		m_velocityPrime->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			context,
			view
		);
	}

	rp->setOutput(velocityTargetSetId, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->endParameters(renderContext);

			WorldRenderPassDeferred velocityPass(
				s_techniqueVelocityWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				false
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

render::handle_t WorldRendererDeferred::setupAmbientOcclusionPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	T_PROFILER_SCOPE(L"World setup ambient occlusion");

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	auto ambientOcclusionTargetSetId = renderGraph.addTransientTargetSet(L"Ambient occlusion", rgtd, nullptr, outputTargetSetId);

	// Add ambient occlusion render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");

	if (m_ambientOcclusion != nullptr)
	{
		render::ImageGraphContext context;
		render::ImageGraphView view;

		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getView();
		view.projection = worldRenderView.getProjection();

		context.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
		context.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);

		m_ambientOcclusion->addPasses(
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
	rp->setOutput(ambientOcclusionTargetSetId, clear, render::TfNone, render::TfColor);

	renderGraph.addPass(rp);
	return ambientOcclusionTargetSetId;
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

	const auto light = m_lights[lightCascadeIndex];
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
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], shadowSettings.farZ));

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

				WorldRenderPassDeferred shadowPass(
					s_techniqueShadow,
					sharedParams,
					shadowRenderView,
					IWorldRenderPass::PfNone,
					false
				);

				T_ASSERT(!renderContext->havePendingDraws());

				for (auto gathered : m_gathered)
					gathered.entityRenderer->build(wc, shadowRenderView, shadowPass, gathered.renderable);
	
				for (auto entityRenderer : m_entityRenderers->get())
					entityRenderer->build(wc, shadowRenderView, shadowPass);

				renderContext->merge(render::RpAll);
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
		const auto light = m_lights[lightAtlasIndex];
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

				WorldRenderPassDeferred shadowPass(
					s_techniqueShadow,
					sharedParams,
					shadowRenderView,
					IWorldRenderPass::PfNone,
					false
				);

				T_ASSERT(!renderContext->havePendingDraws());

				for (auto gathered : m_gathered)
					gathered.entityRenderer->build(wc, shadowRenderView, shadowPass, gathered.renderable);
	
				for (auto entityRenderer : m_entityRenderers->get())
					entityRenderer->build(wc, shadowRenderView, shadowPass);

				renderContext->merge(render::RpAll);

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

void WorldRendererDeferred::setupTileDataPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	TileShaderData* tileShaderData
) const
{
	const Frustum& viewFrustum = worldRenderView.getViewFrustum();

	StaticVector< Vector4, c_maxLightCount > lightPositions;

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

	Vector4 nh = viewFrustum.corners[1] - viewFrustum.corners[0];
	Vector4 nv = viewFrustum.corners[3] - viewFrustum.corners[0];
	Vector4 fh = viewFrustum.corners[5] - viewFrustum.corners[4];
	Vector4 fv = viewFrustum.corners[7] - viewFrustum.corners[4];

	Frustum tileFrustum;
	for (int32_t y = 0; y < ClusterDimXY; ++y)
	{
		Scalar fy = Scalar((float)y) * dy;
		for (int32_t x = 0; x < ClusterDimXY; ++x)
		{
			Scalar fx = Scalar((float)x) * dx;

			Vector4 corners[] =
			{
				// Near
				viewFrustum.corners[0] + nh * fx + nv * fy,					// l t
				viewFrustum.corners[0] + nh * (fx + dx) + nv * fy,			// r t
				viewFrustum.corners[0] + nh * (fx + dx) + nv * (fy + dy),	// r b
				viewFrustum.corners[0] + nh * fx + nv * (fy + dy),			// l b
				// Far
				viewFrustum.corners[4] + fh * fx + fv * fy,					// l t
				viewFrustum.corners[4] + fh * (fx + dx) + fv * fy,			// r t
				viewFrustum.corners[4] + fh * (fx + dx) + fv * (fy + dy),	// r b
				viewFrustum.corners[4] + fh * fx + fv * (fy + dy)			// l b
			};
			tileFrustum.buildFromCorners(corners);

			for (int32_t z = 0; z < ClusterDimZ; ++z)
			{
				Scalar fnz = Scalar((float)z) * dz;
				Scalar ffz = Scalar((float)z + 1.0f) * dz;

				tileFrustum.setNearZ(lerp(viewFrustum.getNearZ(), viewFrustum.getFarZ(), fnz));
				tileFrustum.setFarZ(lerp(viewFrustum.getNearZ(), viewFrustum.getFarZ(), ffz));

				const uint32_t offset = (x + y * ClusterDimXY) * ClusterDimZ + z;

				int32_t count = 0;
				for (uint32_t i = 0; i < m_lights.size(); ++i)
				{
					const auto light = m_lights[i];
					if (light->getLightType() == LightType::LtDirectional)
					{
						tileShaderData[offset].lights[count++] = uint16_t(i);
					}
					else if (light->getLightType() == LightType::LtPoint)
					{
						if (tileFrustum.inside(lightPositions[i], light->getRange()) != Frustum::IrOutside)
							tileShaderData[offset].lights[count++] = uint16_t(i);
					}
					else if (light->getLightType() == LightType::LtSpot)
					{
						tileShaderData[offset].lights[count++] = uint16_t(i);
					}

					if (count >= 4)
						break;
				}
				tileShaderData[offset].lightCount[0] = uint8_t(count);
			}
		}
	}
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
	const auto light = m_lights[lightCascadeIndex];

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

render::handle_t WorldRendererDeferred::setupReflectionsPass(
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

	T_PROFILER_SCOPE(L"World setup reflections");

	// Add Reflections target.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.ignoreStencil = true;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;
	auto reflectionsTargetSetId = renderGraph.addTransientTargetSet(L"Reflections", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add Reflections render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Reflections");

	rp->addInput(gbufferTargetSetId);

	if (m_reflectionsQuality >= Quality::High)
		rp->addInput(visualReadTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(reflectionsTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);
	
	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->endParameters(renderContext);

			WorldRenderPassDeferred reflectionsPass(
				s_techniqueReflectionWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				false
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, reflectionsPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, reflectionsPass);

			renderContext->merge(render::RpAll);
		}
	);

	// Render screenspace reflections.
	if (m_reflectionsQuality >= Quality::High)
	{
		render::ImageGraphContext context;
		render::ImageGraphView view;

		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getView();
		view.projection = worldRenderView.getProjection();
		view.deltaTime = worldRenderView.getDeltaTime();

		context.associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
		context.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
		context.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
		context.associateTextureTargetSet(s_handleInputRoughness, gbufferTargetSetId, 2);

		m_screenReflections->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			context,
			view
		);
	}

	renderGraph.addPass(rp);
	return reflectionsTargetSetId;
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
	int32_t lightCount = (int32_t)m_lights.size();

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

			float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
			float viewFarZ = worldRenderView.getViewFrustum().getFarZ();

			Scalar p11 = projection.get(0, 0);
			Scalar p22 = projection.get(1, 1);

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
			sharedParams->setTextureParameter(s_handleShadowMask, (shadowMaskTargetSet != nullptr) ? shadowMaskTargetSet->getColorTexture(0) : m_whiteTexture);
			sharedParams->setTextureParameter(s_handleShadowMapAtlas, (shadowAtlasTargetSet != nullptr) ? shadowAtlasTargetSet->getDepthTexture() : m_whiteTexture);
			sharedParams->setTextureParameter(s_handleReflectionMap, (reflectionsTargetSet != nullptr) ? reflectionsTargetSet->getColorTexture(0) : m_whiteTexture);
			sharedParams->setBufferViewParameter(s_handleLightSBuffer, m_lightSBuffer->getBufferView());
			sharedParams->setBufferViewParameter(s_handleTileSBuffer, m_tileSBuffer->getBufferView());
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
			WorldRenderPassDeferred irradiancePass(
				s_techniqueIrradianceWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				(bool)(m_irradianceGrid != nullptr)
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, irradiancePass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, irradiancePass);

			renderContext->merge(render::RpAll);

			// Analytical lights; resolve with gbuffer.
			render::Shader::Permutation perm;
			m_lightShader->setCombination(s_handleShadowEnable, (bool)(shadowMaskTargetSet != nullptr), perm);
			m_lightShader->setCombination(s_handleReflectionsEnable, (bool)(reflectionsTargetSet != nullptr), perm);
			m_lightShader->setCombination(s_handleIrradianceEnable, (bool)(m_irradianceGrid != nullptr), perm);
			m_screenRenderer->draw(renderContext, m_lightShader, perm, sharedParams);

			// Module with fog.
			if (dot4(m_fogDistanceAndDensity, Vector4(0.0f, 0.0f, 1.0f, 1.0f)) > FUZZY_EPSILON)
				m_screenRenderer->draw(renderContext, m_fogShader, sharedParams);

			// Forward visuals; not included in GBuffer.
			WorldRenderPassDeferred deferredColorPass(
				s_techniqueDeferredColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfLast,
				(bool)(m_irradianceGrid != nullptr)
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, deferredColorPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, deferredColorPass);

			renderContext->merge(render::RpAll);
		}
	);

	renderGraph.addPass(rp);
}

void WorldRendererDeferred::setupProcessPass(
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
	render::ImageGraphContext context;
	render::ImageGraphView view;

	view.viewFrustum = worldRenderView.getViewFrustum();
	view.viewToLight = Matrix44::identity();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = worldRenderView.getDeltaTime();
	view.time = worldRenderView.getTime();

	context.associateTextureTargetSet(s_handleInputColor, visualWriteTargetSetId, 0);
	context.associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	context.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	context.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	context.associateTextureTargetSet(s_handleInputVelocity, velocityTargetSetId, 0);

	// Expose gamma and exposure.
	context.setFloatParameter(s_handleGamma, m_gamma);
	context.setFloatParameter(s_handleGammaInverse, 1.0f / m_gamma);
	context.setFloatParameter(s_handleExposure, std::pow(2.0f, m_settings.exposure));

	// Expose jitter; in texture space.
	Vector2 rc = jitter(m_count) / worldRenderView.getViewSize();
	Vector2 rp = jitter(m_count - 1) / worldRenderView.getViewSize();
	context.setVectorParameter(s_handleJitter, Vector4(rp.x, -rp.y, rc.x, -rc.y));

	StaticVector< render::ImageGraph*, 5 > processes;
	if (m_motionBlur)
		processes.push_back(m_motionBlur);
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
		bool next = (bool)((i + 1) < processes.size());

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
			context,
			view
		);

		if (next)
			context.associateTextureTargetSet(s_handleInputColor, intermediateTargetSetId, 0);

		renderGraph.addPass(rp);
	}
}

}
