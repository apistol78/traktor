#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Resource/IResourceManager.h"
#include "World/IrradianceGrid.h"
#include "World/WorldContext.h"
#include "World/Deferred/LightRendererDeferred.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Deferred/WorldRenderPassDeferred.h"
#include "World/Entity/GroupEntity.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const int32_t c_maxLightCount = 1024;

const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}");
const resource::Id< render::ImageGraph > c_antiAliasLow(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}");
const resource::Id< render::ImageGraph > c_antiAliasMedium(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{3E1D810B-339A-F742-9345-4ECA00220D57}");
const resource::Id< render::ImageGraph > c_antiAliasHigh(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{0C288028-7BFD-BE46-A25F-F3910BE50319}");
const resource::Id< render::ImageGraph > c_antiAliasUltra(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}");
const resource::Id< render::ImageGraph > c_gammaCorrection(L"{B1E8367D-91DD-D648-A44F-B86492169771}") ; // L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}");
const resource::Id< render::ImageGraph > c_motionBlurPrime(L"{CB34E98B-55C9-E447-BD59-5A1D91DCA88E}") ; // L"{73C2C7DC-BD77-F348-A6B7-06E0EFB633D9}");
const resource::Id< render::ImageGraph > c_motionBlurLow(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}") ; // L"{BDFEFBE0-C5E9-2643-B445-DB02AC5C7687}");
const resource::Id< render::ImageGraph > c_motionBlurMedium(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}") ; // L"{A70CBA02-B75A-E246-A9B6-99B8B2B98D2A}");
const resource::Id< render::ImageGraph > c_motionBlurHigh(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}") ; // L"{E893B98C-90A3-9848-B4F3-3D8C0CE57CE8}");
const resource::Id< render::ImageGraph > c_motionBlurUltra(L"{E813C1A0-D27D-AE4F-9EE4-637529ECCD69}") ; // L"{CD4A0939-233B-2E43-988D-DA6E0DB7A6E6}");
const resource::Id< render::ImageGraph > c_toneMapFixed(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}") ; // L"{838922A0-49CE-6645-8A9C-BA0E71081033}");
const resource::Id< render::ImageGraph > c_toneMapAdaptive(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}") ; // L"{BC4FA128-A976-4023-A422-637581ADFD7E}");

// Techniques
const render::Handle s_techniqueDeferredColor(L"World_DeferredColor");
const render::Handle s_techniqueDeferredGBufferWrite(L"World_DeferredGBufferWrite");
const render::Handle s_techniqueReflectionWrite(L"World_ReflectionWrite");
const render::Handle s_techniqueIrradianceWrite(L"World_IrradianceWrite");
const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");
const render::Handle s_techniqueShadow(L"World_ShadowWrite");

// Global parameters.
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleProjection(L"World_Projection");
const render::Handle s_handleColorMap(L"World_ColorMap");
const render::Handle s_handleOcclusionMap(L"World_OcclusionMap");
const render::Handle s_handleDepthMap(L"World_DepthMap");
const render::Handle s_handleLightMap(L"World_LightMap");
const render::Handle s_handleNormalMap(L"World_NormalMap");
const render::Handle s_handleMiscMap(L"World_MiscMap");
const render::Handle s_handleReflectionMap(L"World_ReflectionMap");
const render::Handle s_handleFogDistanceAndDensity(L"World_FogDistanceAndDensity");
const render::Handle s_handleFogColor(L"World_FogColor");
const render::Handle s_handleLightCount(L"World_LightCount");
const render::Handle s_handleLightSBuffer(L"World_LightSBuffer");
const render::Handle s_handleTileSBuffer(L"World_TileSBuffer");
const render::Handle s_handleIrradianceGridSize(L"World_IrradianceGridSize");
const render::Handle s_handleIrradianceGridSBuffer(L"World_IrradianceGridSBuffer");
const render::Handle s_handleIrradianceGridBoundsMin(L"World_IrradianceGridBoundsMin");
const render::Handle s_handleIrradianceGridBoundsMax(L"World_IrradianceGridBoundsMax");
const render::Handle s_handleGamma(L"World_Gamma");
const render::Handle s_handleGammaInverse(L"World_GammaInverse");
const render::Handle s_handleExposure(L"World_Exposure");

// RenderGraph targets.
const render::Handle s_handleGBuffer(L"GBuffer");
const render::Handle s_handleAmbientOcclusion(L"AmbientOcclusion");
const render::Handle s_handleVelocity(L"Velocity");
const render::Handle s_handleReflections(L"Reflections");
const render::Handle s_handleShadowMapCascade(L"ShadowMapCascade");
const render::Handle s_handleShadowMapAtlas(L"ShadowMapAtlas");
const render::Handle s_handleShadowMask(L"ShadowMask");
const render::Handle s_handleVisual[7] =
{
	render::Handle(L"Visual0"),
	render::Handle(L"Visual1"),
	render::Handle(L"Visual2"),
	render::Handle(L"Visual3"),
	render::Handle(L"Visual4"),
	render::Handle(L"Visual5"),
	render::Handle(L"Visual6")
};

// ImageGraph input textures.
const render::Handle s_handleInputColor(L"InputColor");
const render::Handle s_handleInputDepth(L"InputDepth");
const render::Handle s_handleInputNormal(L"InputNormal");
const render::Handle s_handleInputVelocity(L"InputVelocity");
const render::Handle s_handleInputShadowMap(L"InputShadowMap");

resource::Id< render::ImageGraph > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageGraph >();
	case QuLow:
		return c_ambientOcclusionLow;
	case QuMedium:
		return c_ambientOcclusionMedium;
	case QuHigh:
		return c_ambientOcclusionHigh;
	case QuUltra:
		return c_ambientOcclusionUltra;
	}
}

resource::Id< render::ImageGraph > getAntiAliasId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageGraph >();
	case QuLow:
		return c_antiAliasLow;
	case QuMedium:
		return c_antiAliasMedium;
	case QuHigh:
		return c_antiAliasHigh;
	case QuUltra:
		return c_antiAliasUltra;
	}
}

resource::Id< render::ImageGraph > getMotionBlurId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageGraph >();
	case QuLow:
		return c_motionBlurLow;
	case QuMedium:
		return c_motionBlurMedium;
	case QuHigh:
		return c_motionBlurHigh;
	case QuUltra:
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
	float lights[4];
	float lightCount[4];
};
#pragma pack()

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, IWorldRenderer)

WorldRendererDeferred::WorldRendererDeferred()
:	m_toneMapQuality(QuDisabled)
,	m_motionBlurQuality(QuDisabled)
,	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
,	m_count(0)
{
}

bool WorldRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.toneMapQuality;
	m_motionBlurQuality = desc.motionBlurQuality;
	m_shadowSettings = m_settings.shadowSettings[desc.shadowsQuality];
	m_shadowsQuality = desc.shadowsQuality;
	m_reflectionsQuality = desc.reflectionsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;
	m_sharedDepthStencil = desc.sharedDepthStencil;

	// Allocate frames.
	m_frames.resize(desc.frameCount);

	// Pack fog parameters.
	m_fogDistanceAndDensity = Vector4(
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ
	);
	m_fogColor = m_settings.fogColor;

	// Create shadow screen projection processes.
	if (m_shadowsQuality > QuDisabled)
	{
		if (!resourceManager->bind(m_shadowSettings.maskProject, m_shadowMaskProject))
		{
			log::warning << L"Unable to create shadow project process; shadows disabled." << Endl;
			m_shadowsQuality = QuDisabled;
		}
	}

	// Create ambient occlusion processing.
	if (m_ambientOcclusionQuality > QuDisabled)
	{
		resource::Id< render::ImageGraph > ambientOcclusion = getAmbientOcclusionId(m_ambientOcclusionQuality);
		if (!resourceManager->bind(ambientOcclusion, m_ambientOcclusion))
			log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
	}

	// Create antialias processing.
	if (m_antiAliasQuality > QuDisabled)
	{
		resource::Id< render::ImageGraph > antiAlias = getAntiAliasId(m_antiAliasQuality);
		if (!resourceManager->bind(antiAlias, m_antiAlias))
			log::warning << L"Unable to create antialias process; AA disabled." << Endl;
	}

	// Create "visual" post processing filter.
	if (desc.imageProcessQuality > QuDisabled)
	{
		const auto& visualImageGraph = desc.worldRenderSettings->imageProcess[desc.imageProcessQuality];
		if (!resourceManager->bind(visualImageGraph, m_visual))
			log::warning << L"Unable to create visual post processing; post processing disabled." << Endl;
	}

	// Create gamma correction processing.
	if (
		m_settings.linearLighting &&
		std::abs(desc.gamma - 1.0f) > FUZZY_EPSILON
	)
	{
		if (resourceManager->bind(c_gammaCorrection, m_gammaCorrection))
		{
			m_gammaCorrection->setFloatParameter(s_handleGamma, desc.gamma);
			m_gammaCorrection->setFloatParameter(s_handleGammaInverse, 1.0f / desc.gamma);
		}
		else
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
	}

	// Create motion blur prime processing.
	if (m_motionBlurQuality > QuDisabled)
	{
		if (!resourceManager->bind(c_motionBlurPrime, m_motionBlurPrime))
		{
			log::warning << L"Unable to create motion blur prime process; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}
	}

	// Create motion blur final processing.
	if (m_motionBlurQuality > QuDisabled)
	{
		resource::Id< render::ImageGraph > motionBlur = getMotionBlurId(desc.motionBlurQuality);
		if (!resourceManager->bind(motionBlur, m_motionBlur))
		{
			log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}
	}

	// Create tone map processing.
	if (m_toneMapQuality > QuDisabled)
	{
		resource::Id< render::ImageGraph > toneMap = getToneMapId(m_settings.exposureMode);
		if (resourceManager->bind(toneMap, m_toneMap))
			m_toneMap->setFloatParameter(s_handleExposure, m_settings.exposure);
		else
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = QuDisabled;
		}
	}

	// Allocate light lists.
	for (auto& frame : m_frames)
	{
		AlignedVector< render::StructElement > lightShaderDataStruct;
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, typeRangeRadius)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, position)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, direction)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, color)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight0)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight1)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight2)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight3)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, atlasTransform)));
		T_FATAL_ASSERT(sizeof(LightShaderData) == render::getStructSize(lightShaderDataStruct));

		frame.lightSBuffer = renderSystem->createStructBuffer(
			lightShaderDataStruct,
			render::getStructSize(lightShaderDataStruct) * c_maxLightCount
		);
		if (!frame.lightSBuffer)
			return false;

		AlignedVector< render::StructElement > tileShaderDataStruct;
		tileShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(TileShaderData, lights)));
		tileShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(TileShaderData, lightCount)));
		T_FATAL_ASSERT(sizeof(TileShaderData) == render::getStructSize(tileShaderDataStruct));

		frame.tileSBuffer = renderSystem->createStructBuffer(
			tileShaderDataStruct,
			render::getStructSize(tileShaderDataStruct) * 16 * 16
		);
		if (!frame.tileSBuffer)
			return false;
	}

	// Create light primitive renderer.
	m_lightRenderer = new LightRendererDeferred();
	if (!m_lightRenderer->create(
		resourceManager,
		renderSystem
	))
	{
		log::error << L"Unable to create light primitive renderer." << Endl;
		return false;
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
		float ii = float(i) / m_shadowSettings.cascadingSlices;
		float log = powf(ii, m_shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, m_shadowSettings.farZ, log);
	}
	m_slicePositions[m_shadowSettings.cascadingSlices] = m_shadowSettings.farZ;

	m_entityRenderers = desc.entityRenderers;
	m_rootEntity = new GroupEntity();

	// Create render graph.
	m_renderGraph = new render::RenderGraph(
		renderSystem,
		desc.width,
		desc.height
	);

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

void WorldRendererDeferred::destroy()
{
	for (auto& frame : m_frames)
	{
		safeDestroy(frame.lightSBuffer);
		safeDestroy(frame.tileSBuffer);
	}
	m_frames.clear();

	safeDestroy(m_lightRenderer);
	safeDestroy(m_renderGraph);
	safeDestroy(m_screenRenderer);

	m_irradianceGrid.clear();
}

void WorldRendererDeferred::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererDeferred::build(const WorldRenderView& worldRenderView, render::RenderContext* renderContext)
{
	int32_t frame = m_count % (int32_t)m_frames.size();

	// Gather active lights.
	m_lights.resize(0);
	WorldContext(m_entityRenderers, m_rootEntity).gather(m_rootEntity, m_lights);
	if (m_lights.size() > c_maxLightCount)
		m_lights.resize(c_maxLightCount);

	// Lock light sbuffer; \tbd data is currently written both when setting
	// up render passes and when executing passes.
	LightShaderData* lightShaderData = (LightShaderData*)m_frames[frame].lightSBuffer->lock();
	TileShaderData* tileShaderData = (TileShaderData*)m_frames[frame].tileSBuffer->lock();
	T_FATAL_ASSERT(lightShaderData != nullptr);

	// Write all lights to sbuffer; without shadow map information.
	const Matrix44& view = worldRenderView.getView();
	for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
	{
		const auto& light = m_lights[i];
		auto* lsd = &lightShaderData[i];

		lsd->typeRangeRadius[0] = (float)light.type;
		lsd->typeRangeRadius[1] = light.range;
		lsd->typeRangeRadius[2] = light.radius / 2.0f;
		lsd->typeRangeRadius[3] = 0.0f;

		(view * light.position.xyz1()).storeUnaligned(lsd->position);
		(view * light.direction.xyz0()).storeUnaligned(lsd->direction);
		light.color.storeUnaligned(lsd->color);

		Vector4::zero().storeUnaligned(lsd->viewToLight0);
		Vector4::zero().storeUnaligned(lsd->viewToLight1);
		Vector4::zero().storeUnaligned(lsd->viewToLight2);
		Vector4::zero().storeUnaligned(lsd->viewToLight3);
	}

	// Find directional light for cascade shadow map.
	int32_t lightCascadeIndex = -1;
	if (m_shadowsQuality != QuDisabled)
	{
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto& light = m_lights[i];
			if (light.castShadow && light.type == LtDirectional)
			{
				lightCascadeIndex = i;
				break;
			}
		}
	}

	// Find spot lights for atlas shadow map.
	StaticVector< int32_t, 16 > lightAtlasIndices;
	if (m_shadowsQuality != QuDisabled)
	{
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto& light = m_lights[i];
			if (light.castShadow && light.type == LtSpot)
				lightAtlasIndices.push_back(i);
		}
	}

	buildGBuffer(worldRenderView);
	buildVelocity(worldRenderView);
	buildAmbientOcclusion(worldRenderView);
	buildCascadeShadowMap(worldRenderView, lightCascadeIndex, lightShaderData);
	buildAtlasShadowMap(worldRenderView, lightAtlasIndices, lightShaderData);
	buildTileData(worldRenderView, tileShaderData);
	buildShadowMask(worldRenderView, lightCascadeIndex);
	buildReflections(worldRenderView);
	buildVisual(worldRenderView, frame);
	buildProcess(worldRenderView);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context through render graph.
	m_renderGraph->build(renderContext);

	// Unlock light sbuffers.
	m_frames[frame].tileSBuffer->unlock();
	m_frames[frame].lightSBuffer->unlock();

	// Flush attached entities.
	m_rootEntity->removeAllEntities();
	m_count++;
}

void WorldRendererDeferred::buildGBuffer(const WorldRenderView& worldRenderView) const
{
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 4;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR16F;		// Depth (R)
	rgtd.targets[1].colorFormat = render::TfR16G16F;	// Normals (RG)
	rgtd.targets[2].colorFormat = render::TfR11G11B10F;	// Metalness (R), Roughness (G), Specular (B)
	rgtd.targets[3].colorFormat = render::TfR11G11B10F;	// Surface color (RGB)
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
	m_renderGraph->addTargetSet(s_handleGBuffer, rgtd, m_sharedDepthStencil);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");

	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth;
	clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);	// depth
	clear.colors[1] = Color4f(0.0f, 0.0f, 1.0f, 0.0f);	// normal
	clear.colors[2] = Color4f(0.0f, 1.0f, 0.0f, 1.0f);	// misc
	clear.colors[3] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);	// surface
	clear.depth = 1.0f;	
	rp->setOutput(s_handleGBuffer, clear);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
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
			wc.build(worldRenderView, gbufferPass, m_rootEntity);
			wc.flush(worldRenderView, gbufferPass);
			renderContext->merge(render::RpAll);
		}
	);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildVelocity(const WorldRenderView& worldRenderView) const
{
	if (m_motionBlurQuality == QuDisabled)
		return;

	// Add Velocity target set.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR16G16F;
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
	m_renderGraph->addTargetSet(s_handleVelocity, rgtd, m_sharedDepthStencil);

	// Add Velocity render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Velocity");
	
	if (m_motionBlurPrime)
	{
		render::ImageGraphContext cx(m_screenRenderer);
		cx.associateTextureTargetSet(s_handleInputDepth, s_handleGBuffer, 0);
		m_motionBlurPrime->addPasses(m_renderGraph, rp, cx);
	}

	rp->setOutput(s_handleVelocity);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
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

			wc.build(worldRenderView, velocityPass, m_rootEntity);
			wc.flush(worldRenderView, velocityPass);
			renderContext->merge(render::RpAll);
		}
	);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildAmbientOcclusion(const WorldRenderView& worldRenderView) const
{
	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
	m_renderGraph->addTargetSet(s_handleAmbientOcclusion, rgtd, m_sharedDepthStencil);

	// Add ambient occlusion render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");

	if (m_ambientOcclusion != nullptr)
	{
		render::ImageGraphParams ipd;
		ipd.viewFrustum = worldRenderView.getViewFrustum();
		ipd.view = worldRenderView.getView();
		ipd.projection = worldRenderView.getProjection();

		render::ImageGraphContext cx(m_screenRenderer);
		cx.associateTextureTargetSet(s_handleInputDepth, s_handleGBuffer, 0);
		cx.associateTextureTargetSet(s_handleInputNormal, s_handleGBuffer, 1);
		cx.setParams(ipd);

		m_ambientOcclusion->addPasses(m_renderGraph, rp, cx);
	}

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(s_handleAmbientOcclusion, clear);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildCascadeShadowMap(const WorldRenderView& worldRenderView, int32_t lightCascadeIndex, LightShaderData* lightShaderData) const
{
	if (lightCascadeIndex < 0)
		return;

	const auto& shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Add cascading shadow map target.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 0;
	rgtd.width = shadowSettings.resolution;
	rgtd.height = shadowSettings.cascadingSlices * shadowSettings.resolution;
	rgtd.createDepthStencil = true;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.usingDepthStencilAsTexture = true;
	rgtd.ignoreStencil = true;
	m_renderGraph->addTargetSet(s_handleShadowMapCascade, rgtd);

	// Add cascading shadow map render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow cascade");

	render::Clear clear;
	clear.mask = render::CfDepth;
	clear.depth = 1.0f;
	rp->setOutput(s_handleShadowMapCascade, clear);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			const auto& light = m_lights[lightCascadeIndex];
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
					light.position,
					light.direction,
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
				wc.build(shadowRenderView, shadowPass, m_rootEntity);
				wc.flush(shadowRenderView, shadowPass);
				renderContext->merge(render::RpAll);
			}
		}
	);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildAtlasShadowMap(const WorldRenderView& worldRenderView, const StaticVector< int32_t, 16 >& lightAtlasIndices, LightShaderData* lightShaderData) const
{
	if (lightAtlasIndices.empty())
		return;

	const auto shadowSettings = m_settings.shadowSettings[m_shadowsQuality];

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Add atlas shadow map target.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 0;
	rgtd.width =
	rgtd.height = 4096;
	rgtd.createDepthStencil = true;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.usingDepthStencilAsTexture = true;
	rgtd.ignoreStencil = true;
	m_renderGraph->addTargetSet(s_handleShadowMapAtlas, rgtd);

	// Add atlas shadow map render pass.
	int32_t atlasIndex = 0;
	for (int32_t lightAtlasIndex : lightAtlasIndices)
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow atlas");

		render::Clear clear;
		clear.mask = render::CfDepth;
		clear.depth = 1.0f;
		rp->setOutput(s_handleShadowMapAtlas, clear);

		rp->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				WorldContext wc(
					m_entityRenderers,
					renderContext,
					m_rootEntity
				);

				const auto& light = m_lights[lightAtlasIndex];
				auto* lsd = &lightShaderData[lightAtlasIndex];

				// Calculate shadow map projection.
				Matrix44 shadowLightView;
				Matrix44 shadowLightProjection;
				Frustum shadowFrustum;

				shadowFrustum.buildPerspective(
					light.radius,
					1.0f,
					0.1f,
					light.range
				);

				shadowLightProjection = perspectiveLh(light.radius, 1.0f, 0.1f, light.range);

				Vector4 lightAxisX, lightAxisY, lightAxisZ;
				lightAxisZ = -light.direction.xyz0().normalized();
				lightAxisX = cross(viewInverse.axisZ(), lightAxisZ).normalized();
				lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

				shadowLightView = Matrix44(
					lightAxisX,
					lightAxisY,
					lightAxisZ,
					light.position.xyz1()
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
				wc.build(shadowRenderView, shadowPass, m_rootEntity);
				wc.flush(shadowRenderView, shadowPass);
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

		m_renderGraph->addPass(rp);

		++atlasIndex;
	}		
}

void WorldRendererDeferred::buildTileData(const WorldRenderView& worldRenderView, TileShaderData* tileShaderData) const
{
	const Frustum& viewFrustum = worldRenderView.getViewFrustum();

	// Update tile data.
	const float dx = 1.0f / 16.0f;
	const float dy = 1.0f / 16.0f;

	Vector4 nh = viewFrustum.corners[1] - viewFrustum.corners[0];
	Vector4 nv = viewFrustum.corners[3] - viewFrustum.corners[0];
	Vector4 fh = viewFrustum.corners[5] - viewFrustum.corners[4];
	Vector4 fv = viewFrustum.corners[7] - viewFrustum.corners[4];

	Frustum tileFrustum;
	for (int32_t y = 0; y < 16; ++y)
	{
		float fy = float(y) * dy;
		for (int32_t x = 0; x < 16; ++x)
		{
			float fx = float(x) * dx;

			Vector4 corners[] =
			{
				// Near
				viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy),				// l t
				viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy),		// r t
				viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy + dy),	// r b
				viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy + dy),		// l b
				// Far
				viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy),				// l t
				viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy),		// r t
				viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy + dy),	// r b
				viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy + dy)			// l b
			};

			tileFrustum.buildFromCorners(corners);

			int32_t count = 0;
			for (uint32_t i = 0; i < m_lights.size(); ++i)
			{
				const Light& light = m_lights[i];

				if (light.type == LtDirectional)
				{
					tileShaderData[x + y * 16].lights[count++] = float(i);
				}
				else if (light.type == LtPoint)
				{
					Vector4 lvp = worldRenderView.getView() * light.position.xyz1();
					if (tileFrustum.inside(lvp, Scalar(light.range)) != Frustum::IrOutside)
						tileShaderData[x + y * 16].lights[count++] = float(i);
				}
				else if (light.type == LtSpot)
				{
					tileShaderData[x + y * 16].lights[count++] = float(i);
				}

				if (count >= 4)
					break;
			}
			tileShaderData[x + y * 16].lightCount[0] = float(count);
		}
	}
}

void WorldRendererDeferred::buildShadowMask(const WorldRenderView& worldRenderView, int32_t lightCascadeIndex) const
{
	if (m_shadowsQuality == QuDisabled || lightCascadeIndex < 0)
		return;

	const auto shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);
	const auto& light = m_lights[lightCascadeIndex];

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	// Add screen space shadow mask target.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 1;
	rgtd.width = 0;
	rgtd.height = 0;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.ignoreStencil = true;
	rgtd.targets[0].colorFormat = render::TfR8;
	rgtd.screenWidthDenom = m_shadowSettings.maskDenominator;
	rgtd.screenHeightDenom = m_shadowSettings.maskDenominator;
	m_renderGraph->addTargetSet(s_handleShadowMask, rgtd, m_sharedDepthStencil);

	// Add screen space shadow mask render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow mask");

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
			light.position,
			light.direction,
			sliceViewFrustum,
			shadowSettings.farZ,
			shadowSettings.quantizeProjection,
			shadowLightView,
			shadowLightProjection,
			shadowFrustum
		);

		render::ImageGraphParams ipd;
		ipd.viewFrustum = worldRenderView.getViewFrustum();
		ipd.viewToLight = shadowLightProjection * shadowLightView * viewInverse;
		ipd.projection = worldRenderView.getProjection();
		ipd.sliceCount = m_shadowSettings.cascadingSlices;
		ipd.sliceIndex = slice;
		ipd.sliceNearZ = zn;
		ipd.sliceFarZ = zf;
		ipd.shadowFarZ = m_shadowSettings.farZ;
		ipd.shadowMapBias = m_shadowSettings.bias + slice * m_shadowSettings.biasCoeff;
		ipd.shadowMapUvTransform = Vector4(
			0.0f, (float)slice / m_shadowSettings.cascadingSlices,
			1.0f, 1.0f / m_shadowSettings.cascadingSlices
		);
		ipd.deltaTime = 0.0f;
		ipd.time = 0.0f;

		render::ImageGraphContext cx(m_screenRenderer);
		cx.associateTextureTargetSetDepth(s_handleInputShadowMap, s_handleShadowMapCascade);
		cx.associateTextureTargetSet(s_handleInputDepth, s_handleGBuffer, 0);
		cx.associateTextureTargetSet(s_handleInputNormal, s_handleGBuffer, 1);
		cx.setParams(ipd);

		m_shadowMaskProject->addPasses(m_renderGraph, rp, cx);
	}

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(s_handleShadowMask, clear);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildReflections(const WorldRenderView& worldRenderView) const
{
	if (m_reflectionsQuality == QuDisabled)
		return;

	// Add Reflections target.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.ignoreStencil = true;
#if !defined(__ANDROID__) && !defined(__IOS__)
	rgtd.targets[0].colorFormat = render::TfR16G16B16A16F;
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
#else
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;
	rgtd.screenWidthDenom = 2;
	rgtd.screenHeightDenom = 2;
#endif
	m_renderGraph->addTargetSet(s_handleReflections, rgtd, m_sharedDepthStencil);

	// Add Reflections render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Reflections");

	rp->addInput(s_handleGBuffer);

	// if (m_reflectionsQuality >= QuHigh)
	// 	rp->addInput(s_handleVisual[0], 0, true);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(s_handleReflections, clear);
	
	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto gbufferTargetSet = renderGraph.getTargetSet(s_handleGBuffer);
			auto visualTargetSet = renderGraph.getTargetSet(s_handleVisual[0]);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleColorMap, gbufferTargetSet->getColorTexture(3));
			sharedParams->endParameters(renderContext);

			WorldRenderPassDeferred reflectionsPass(
				s_techniqueReflectionWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				false
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, reflectionsPass, m_rootEntity);
			wc.flush(worldRenderView, reflectionsPass);
			renderContext->merge(render::RpAll);

			// Render screenspace reflections.
			// if (m_reflectionsQuality >= QuHigh)
			// {
			// 	m_lightRenderer->renderReflections(
			// 		renderContext,
			// 		worldRenderView.getProjection(),
			// 		worldRenderView.getView(),
			// 		worldRenderView.getLastView(),
			// 		visualTargetSet->getColorTexture(0),	// \tbd using last frame copy without reprojection...
			// 		gbufferTargetSet->getColorTexture(0),	// depth
			// 		gbufferTargetSet->getColorTexture(1),	// normals
			// 		gbufferTargetSet->getColorTexture(2)	// metalness, roughness and specular
			// 	);
			// }
		}
	);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildVisual(const WorldRenderView& worldRenderView, int32_t frame) const
{
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	int32_t lightCount = (int32_t)m_lights.size();

	// Add visual[0] target set.
	render::RenderGraphTargetSetDesc rgtd = {};
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
	m_renderGraph->addTargetSet(s_handleVisual[0], rgtd, m_sharedDepthStencil);

	// Add visual[0] render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(s_handleGBuffer);
	rp->addInput(s_handleAmbientOcclusion);
	rp->addInput(s_handleReflections);

	if (shadowsEnable)
	{
		rp->addInput(s_handleShadowMask);
		rp->addInput(s_handleShadowMapAtlas);
	}

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(s_handleVisual[0], clear);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto gbufferTargetSet = renderGraph.getTargetSet(s_handleGBuffer);
			auto ambientOcclusionTargetSet = renderGraph.getTargetSet(s_handleAmbientOcclusion);
			auto reflectionsTargetSet = renderGraph.getTargetSet(s_handleReflections);
			auto shadowMaskTargetSet = renderGraph.getTargetSet(s_handleShadowMask);
			auto shadowAtlasTargetSet = renderGraph.getTargetSet(s_handleShadowMapAtlas);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setFloatParameter(s_handleLightCount, (float)lightCount);
			sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, m_fogDistanceAndDensity);
			sharedParams->setVectorParameter(s_handleFogColor, m_fogColor);
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleColorMap, gbufferTargetSet->getColorTexture(3));
			sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			sharedParams->setStructBufferParameter(s_handleLightSBuffer, m_frames[frame].lightSBuffer);
			sharedParams->setStructBufferParameter(s_handleTileSBuffer, m_frames[frame].tileSBuffer);
			if (m_irradianceGrid)
			{
				const auto size = m_irradianceGrid->getSize();
				sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0], (float)size[1], (float)size[2], 0.0f));
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
				sharedParams->setStructBufferParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer());
			}
			sharedParams->endParameters(renderContext);

			// Irradiance
			WorldRenderPassDeferred irradiancePass(
				s_techniqueIrradianceWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				(bool)m_irradianceGrid
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, irradiancePass, m_rootEntity);
			wc.flush(worldRenderView, irradiancePass);
			renderContext->merge(render::RpAll);

			// Analytical lights; resolve with gbuffer.
			m_lightRenderer->renderLights(
				renderContext,
				worldRenderView.getTime(),
				lightCount,
				worldRenderView.getProjection(),
				worldRenderView.getView(),
				m_frames[frame].lightSBuffer,
				m_frames[frame].tileSBuffer,
				m_irradianceGrid,
				gbufferTargetSet->getColorTexture(0),	// depth
				gbufferTargetSet->getColorTexture(1),	// normals
				gbufferTargetSet->getColorTexture(2),	// metalness/roughness
				gbufferTargetSet->getColorTexture(3),	// surface color
				ambientOcclusionTargetSet->getColorTexture(0),	// ambient occlusion
				shadowMaskTargetSet != nullptr ? shadowMaskTargetSet->getColorTexture(0) : nullptr,		// shadow mask
				shadowAtlasTargetSet != nullptr ? shadowAtlasTargetSet->getDepthTexture() : nullptr,	// shadow map atlas,
				reflectionsTargetSet != nullptr ? reflectionsTargetSet->getColorTexture(0) : nullptr	// reflection map
			);

			// Fog
			if (dot4(m_fogDistanceAndDensity, Vector4(0.0f, 0.0f, 1.0f, 1.0f)) > FUZZY_EPSILON)
			{
				m_lightRenderer->renderFog(
					renderContext,
					worldRenderView.getProjection(),
					worldRenderView.getView(),
					m_fogDistanceAndDensity,
					m_fogColor,
					gbufferTargetSet->getColorTexture(0),
					gbufferTargetSet->getColorTexture(1),
					gbufferTargetSet->getColorTexture(2),
					gbufferTargetSet->getColorTexture(3)
				);
			}

			// Forward visuals; not included in GBuffer.
			WorldRenderPassDeferred deferredColorPass(
				s_techniqueDeferredColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfLast,
				m_settings.fog,
				gbufferTargetSet->getColorTexture(0) != nullptr
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, deferredColorPass, m_rootEntity);
			wc.flush(worldRenderView, deferredColorPass);
			renderContext->merge(render::RpAll);
		}
	);

	m_renderGraph->addPass(rp);
}

void WorldRendererDeferred::buildProcess(const WorldRenderView& worldRenderView) const
{
	render::ImageGraphParams ipd;
	ipd.viewFrustum = worldRenderView.getViewFrustum();
	ipd.viewToLight = Matrix44::identity();
	ipd.view = worldRenderView.getView();
	ipd.projection = worldRenderView.getProjection();
	ipd.deltaTime = 1.0f / 60.0f;				

	render::ImageGraphContext cx(m_screenRenderer);
	cx.associateTextureTargetSet(s_handleInputColor, s_handleVisual[0], 0);
	cx.associateTextureTargetSet(s_handleInputDepth, s_handleGBuffer, 0);
	cx.associateTextureTargetSet(s_handleInputNormal, s_handleGBuffer, 1);
	cx.associateTextureTargetSet(s_handleInputVelocity, s_handleVelocity, 0);
	cx.setParams(ipd);

	// Collect active image graphs.
	StaticVector< render::ImageGraph*, 5 > processes;
	if (m_motionBlur)
		processes.push_back(m_motionBlur);
	if (m_toneMap)
		processes.push_back(m_toneMap);
	if (m_visual)
		processes.push_back(m_visual);
	if (m_gammaCorrection)
		processes.push_back(m_gammaCorrection);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);

	// Add visual[N] and image graph render targets.
	for (size_t i = 1; i < processes.size(); ++i)
	{
		// Add visual[N] render target.
		render::RenderGraphTargetSetDesc rgtd = {};
		rgtd.count = 1;
		rgtd.createDepthStencil = false;
		rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
		rgtd.targets[0].colorFormat = render::TfR11G11B10F;
		rgtd.screenWidthDenom = 1;
		rgtd.screenHeightDenom = 1;
		m_renderGraph->addTargetSet(s_handleVisual[i], rgtd, m_sharedDepthStencil);

		// Add image graph render targets.
		processes[i]->addTargetSets(m_renderGraph);
	}

	// Add visual[N] render passes.
	for (size_t i = 0; i < processes.size(); ++i)
	{
		auto process = processes[i];
		bool next = (bool)((i + 1) < processes.size());

		Ref< render::RenderPass > rp = new render::RenderPass(L"Process");

		if (next)
			rp->setOutput(s_handleVisual[i + 1]);

		process->addPasses(m_renderGraph, rp, cx);

		m_renderGraph->addPass(rp);
	}
}

	}
}
