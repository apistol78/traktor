#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/ImageProcessData.h"
#include "Resource/IResourceManager.h"
#include "World/WorldContext.h"
#include "World/Entity/GroupEntity.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const int32_t c_maxLightCount = 1024;

const resource::Id< render::ImageProcessData > c_ambientOcclusionLow(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionMedium(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionHigh(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionUltra(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}");
const resource::Id< render::ImageProcessData > c_antiAliasNone(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}");
const resource::Id< render::ImageProcessData > c_antiAliasLow(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}");
const resource::Id< render::ImageProcessData > c_antiAliasMedium(L"{3E1D810B-339A-F742-9345-4ECA00220D57}");
const resource::Id< render::ImageProcessData > c_antiAliasHigh(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}");
const resource::Id< render::ImageProcessData > c_antiAliasUltra(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}");
const resource::Id< render::ImageProcessData > c_gammaCorrection(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}");
const resource::Id< render::ImageProcessData > c_toneMapFixed(L"{838922A0-49CE-6645-8A9C-BA0E71081033}");
const resource::Id< render::ImageProcessData > c_toneMapAdaptive(L"{BC4FA128-A976-4023-A422-637581ADFD7E}");

// Techniques
const render::Handle s_techniqueForwardColor(L"World_ForwardColor");
const render::Handle s_techniqueForwardGBufferWrite(L"World_ForwardGBufferWrite");
const render::Handle s_techniqueShadow(L"World_ShadowWrite");

// Global parameters.
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleProjection(L"World_Projection");

// Render graph.
const render::Handle s_handleGBuffer(L"GBuffer");
const render::Handle s_handleAmbientOcclusion(L"AmbientOcclusion");
const render::Handle s_handleShadowMapCascade(L"ShadowMapCascade");
const render::Handle s_handleShadowMapAtlas(L"ShadowMapAtlas");
const render::Handle s_handleVisual[] =
{
	render::Handle(L"Visual1"),
	render::Handle(L"Visual2"),
	render::Handle(L"Visual3"),
	render::Handle(L"Visual4"),
	render::Handle(L"Visual5"),
	render::Handle(L"Visual6")
};

resource::Id< render::ImageProcessData > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageProcessData >();
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

resource::Id< render::ImageProcessData > getAntiAliasId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return c_antiAliasNone;
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

resource::Id< render::ImageProcessData > getToneMapId(WorldRenderSettings::ExposureMode exposureMode)
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererForward", 0, WorldRendererForward, IWorldRenderer)

WorldRendererForward::WorldRendererForward()
:	m_toneMapQuality(QuDisabled)
,	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
,	m_count(0)
{
}

bool WorldRendererForward::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	render::RenderTargetSetCreateDesc rtscd;
	render::RenderTargetAutoSize rtas;
	render::Clear clear;

	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.toneMapQuality;
	m_shadowsQuality = desc.shadowsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;

	// Allocate frames, one for each queued frame.
	m_frames.resize(desc.frameCount);

	// Create ambient occlusion processing.
	{
		resource::Id< render::ImageProcessData > ambientOcclusionId = getAmbientOcclusionId(m_ambientOcclusionQuality);
		resource::Proxy< render::ImageProcessData > ambientOcclusion;

		if (ambientOcclusionId)
		{
			if (!resourceManager->bind(ambientOcclusionId, ambientOcclusion))
				log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
		}

		if (ambientOcclusion)
		{
			m_ambientOcclusion = new render::ImageProcess();
			if (!m_ambientOcclusion->create(
				ambientOcclusion,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
				m_ambientOcclusion = nullptr;
			}
		}
	}

	// Create antialias processing.
	{
		resource::Id< render::ImageProcessData > antiAliasId = getAntiAliasId(m_antiAliasQuality);
		resource::Proxy< render::ImageProcessData > antiAlias;

		if (antiAliasId != c_antiAliasNone)
		{
			if (!resourceManager->bind(antiAliasId, antiAlias))
				log::warning << L"Unable to create antialias process; AA disabled." << Endl;
		}

		if (antiAlias)
		{
			m_antiAlias = new render::ImageProcess();
			if (!m_antiAlias->create(
				antiAlias,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create antialias process; AA disabled." << Endl;
				m_antiAlias = nullptr;
			}
		}
	}

	// Create "visual" post processing filter.
	{
		const resource::Id< render::ImageProcessData >& imageProcessSettings = desc.worldRenderSettings->imageProcess[desc.imageProcessQuality];
		if (imageProcessSettings)
		{
			resource::Proxy< render::ImageProcessData > imageProcess;
			if (!resourceManager->bind(imageProcessSettings, imageProcess))
				log::warning << L"Unable to create visual post processing image filter; post processing disabled." << Endl;

			if (imageProcess)
			{
				m_visualImageProcess = new render::ImageProcess();
				if (!m_visualImageProcess->create(
					imageProcess,
					nullptr,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height,
					desc.allTargetsPersistent
				))
				{
					log::warning << L"Unable to create visual post processing; post processing disabled." << Endl;
					m_visualImageProcess = nullptr;
				}
			}
		}
	}

	// Create gamma correction processing.
	if (
		m_settings.linearLighting &&
		std::abs(desc.gamma - 1.0f) > FUZZY_EPSILON
	)
	{
		resource::Proxy< render::ImageProcessData > gammaCorrection;
		if (!resourceManager->bind(c_gammaCorrection, gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;

		if (gammaCorrection)
		{
			m_gammaCorrectionImageProcess = new render::ImageProcess();
			if (m_gammaCorrectionImageProcess->create(
				gammaCorrection,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				m_gammaCorrectionImageProcess->setFloatParameter(render::getParameterHandle(L"World_Gamma"), desc.gamma);
				m_gammaCorrectionImageProcess->setFloatParameter(render::getParameterHandle(L"World_GammaInverse"), 1.0f / desc.gamma);
			}
			else
			{
				log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
				m_gammaCorrectionImageProcess = nullptr;
			}
		}
	}

	// Create tone map processing.
	if (m_toneMapQuality > QuDisabled)
	{
		resource::Id< render::ImageProcessData > toneMapId = getToneMapId(m_settings.exposureMode);
		resource::Proxy< render::ImageProcessData > toneMap;

		if (!resourceManager->bind(toneMapId, toneMap))
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = QuDisabled;
		}

		if (toneMap)
		{
			m_toneMapImageProcess = new render::ImageProcess();
			if (m_toneMapImageProcess->create(
				toneMap,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
				m_toneMapImageProcess->setFloatParameter(
					render::getParameterHandle(L"World_Exposure"),
					m_settings.exposure
				);
			else
			{
				log::warning << L"Unable to create tone map process; tone mapping disabled." << Endl;
				m_toneMapImageProcess = nullptr;
				m_toneMapQuality = QuDisabled;
			}
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
	}

	// Determine slice distances.
	const auto& shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / shadowSettings.cascadingSlices;
		float log = powf(ii, shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
	}
	m_slicePositions[shadowSettings.cascadingSlices] = shadowSettings.farZ;

	m_entityRenderers = desc.entityRenderers;
	m_rootEntity = new GroupEntity();

	// Create render graph.
	m_renderGraph = new render::RenderGraph(
		renderSystem,
		desc.width,
		desc.height
	);
	
	// GBuffer
	rtscd.count = 2;
	rtscd.multiSample = desc.multiSample;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.targets[0].format = render::TfR16F;		// Depth (R)
	rtscd.targets[1].format = render::TfR16G16F;	// Normals (RG)
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	m_renderGraph->addRenderTarget(s_handleGBuffer, rtscd, rtas);

	// Ambient occlusion.
	rtscd.count = 1;
	rtscd.multiSample = desc.multiSample;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.targets[0].format = render::TfR8;			// Ambient occlusion (R)
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	m_renderGraph->addRenderTarget(s_handleAmbientOcclusion, rtscd, rtas);

	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	if (shadowsEnable)
	{
		// Cascading shadow map.
		rtscd.count = 0;
		rtscd.width = 1024;
		rtscd.height = m_settings.shadowSettings[m_shadowsQuality].cascadingSlices * 1024;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		m_renderGraph->addRenderTarget(s_handleShadowMapCascade, rtscd);

		// Atlas shadow map.
		rtscd.count = 0;
		rtscd.width =
		rtscd.height = 4096;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		m_renderGraph->addRenderTarget(s_handleShadowMapAtlas, rtscd);
	}

	// Visual
	rtscd.count = 1;
	rtscd.multiSample = desc.multiSample;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.targets[0].format = render::TfR11G11B10F;
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	for (int32_t i = 0; i < sizeof_array(s_handleVisual); ++i)
		m_renderGraph->addRenderTarget(s_handleVisual[i], rtscd, rtas);

	// Allocate render contexts.
	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(1 * 1024 * 1024);

	m_count = 0;
	return true;
}

void WorldRendererForward::destroy()
{
	for (auto& frame : m_frames)
		safeDestroy(frame.lightSBuffer);
	m_frames.clear();

	safeDestroy(m_toneMapImageProcess);
	safeDestroy(m_gammaCorrectionImageProcess);
	safeDestroy(m_visualImageProcess);
	safeDestroy(m_renderGraph);
}

void WorldRendererForward::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererForward::build(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	// Reset render context by flushing it.
	m_frames[frame].renderContext->flush();

	// Gather active lights.
	m_lights.resize(0);
	wc.gather(
		worldRenderView,
		m_rootEntity,
		m_lights
	);

	// Discard excessive lights.
	if (m_lights.size() > c_maxLightCount)
		m_lights.resize(c_maxLightCount);

	// Lock light sbuffer; \tbd data is currently written both when setting
	// up render passes and when executing passes.
	LightShaderData* lightShaderData = (LightShaderData*)m_frames[frame].lightSBuffer->lock();
	T_FATAL_ASSERT(lightShaderData != nullptr);

	// \tbd Flush all entity renderers first, only used by probes atm and need to render to targets.
	// Until we have RenderGraph properly implemented we need to make sure
	// rendering probes doesn't nest render passes.
	wc.flush();
	wc.getRenderContext()->merge(render::RpAll);

	// Add each pass to render graph.
	buildGBuffer(worldRenderView);
	buildAmbientOcclusion(worldRenderView);
	buildLights(worldRenderView, frame, lightShaderData);
	buildVisual(worldRenderView, frame);
	buildProcess(worldRenderView);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context through render graph.
	m_renderGraph->build(
		m_frames[frame].renderContext
	);

	// Unlock light sbuffer.
	m_frames[frame].lightSBuffer->unlock();

	m_count++;
	m_rootEntity->removeAllEntities();
}

void WorldRendererForward::render(render::IRenderView* renderView, int32_t frame)
{
	m_frames[frame].renderContext->render(renderView);
}

render::ImageProcess* WorldRendererForward::getVisualImageProcess()
{
	return m_visualImageProcess;
}

void WorldRendererForward::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
	// if (m_visualTargetSet)
	// 	outTargets.push_back(render::DebugTarget(L"Visual", render::DtvDefault, m_visualTargetSet->getColorTexture(0)));

	// if (m_intermediateTargetSet)
	// 	outTargets.push_back(render::DebugTarget(L"Intermediate", render::DtvDefault, m_intermediateTargetSet->getColorTexture(0)));

	// if (m_gbufferTargetSet)
	// {
	// 	outTargets.push_back(render::DebugTarget(L"GBuffer depth", render::DtvViewDepth, m_gbufferTargetSet->getColorTexture(0)));
	// 	outTargets.push_back(render::DebugTarget(L"GBuffer normals", render::DtvNormals, m_gbufferTargetSet->getColorTexture(1)));
	// 	outTargets.push_back(render::DebugTarget(L"GBuffer AO", render::DtvDefault, m_gbufferTargetSet->getColorTexture(2)));
	// }

	// if (m_shadowCascadeTargetSet)
	// 	outTargets.push_back(render::DebugTarget(L"Shadow map (cascade)", render::DtvShadowMap, m_shadowCascadeTargetSet->getDepthTexture()));

	// if (m_shadowAtlasTargetSet)
	// 	outTargets.push_back(render::DebugTarget(L"Shadow map (atlas)", render::DtvShadowMap, m_shadowAtlasTargetSet->getDepthTexture()));

	// if (m_ambientOcclusion)
	// 	m_ambientOcclusion->getDebugTargets(outTargets);

	// if (m_antiAlias)
	// 	m_antiAlias->getDebugTargets(outTargets);

	// if (m_visualImageProcess)
	// 	m_visualImageProcess->getDebugTargets(outTargets);

	// if (m_gammaCorrectionImageProcess)
	// 	m_gammaCorrectionImageProcess->getDebugTargets(outTargets);

	// if (m_toneMapImageProcess)
	// 	m_toneMapImageProcess->getDebugTargets(outTargets);
}

void WorldRendererForward::buildGBuffer(const WorldRenderView& worldRenderView)
{
	m_renderGraph->addPass(
		L"GBuffer",
		[&](render::RenderPassBuilder& builder)
		{
			const float clearZ = m_settings.viewFarZ;

			render::Clear clear;
			clear.mask = render::CfColor | render::CfDepth;
			clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);
			clear.colors[1] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			clear.depth = 1.0f;	

			builder.setOutput(s_handleGBuffer, clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->endParameters(renderContext);

			WorldRenderPassForward pass(
				s_techniqueForwardGBufferWrite,
				sharedParams,
				IWorldRenderPass::PfFirst,
				worldRenderView.getView(),
				nullptr,
				nullptr,
				nullptr
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, pass, m_rootEntity);
			wc.flush(worldRenderView, pass);
			renderContext->merge(render::RpAll);		
		}
	);
}

void WorldRendererForward::buildAmbientOcclusion(const WorldRenderView& worldRenderView)
{
	m_renderGraph->addPass(
		L"Ambient occlusion",
		[&](render::RenderPassBuilder& builder)
		{
			if (m_ambientOcclusion != nullptr)
				builder.addInput(s_handleGBuffer);

			render::Clear clear;
			clear.mask = render::CfColor;
			clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);

			builder.setOutput(s_handleAmbientOcclusion, clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			if (m_ambientOcclusion == nullptr)
				return;

			auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);

			render::ImageProcessStep::Instance::RenderParams params;
			params.viewFrustum = worldRenderView.getViewFrustum();
			params.view = worldRenderView.getView();
			params.projection = worldRenderView.getProjection();
			params.deltaTime = 0.0f;

			m_ambientOcclusion->build(
				renderContext,
				nullptr,	// color
				gbufferTargetSet->getColorTexture(0),	// depth
				gbufferTargetSet->getColorTexture(1),	// normal
				nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
		}
	);
}

void WorldRendererForward::buildLights(const WorldRenderView& worldRenderView, int32_t frame, LightShaderData* lightShaderData)
{
	const UniformShadowProjection shadowProjection(1024);
	const auto shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	
	// Find cascade shadow light.
	int32_t lightCascadeIndex = -1;
	if (shadowsEnable)
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

	// Find atlas shadow lights.
	StaticVector< int32_t, 16 > lightAtlasIndices;
	if (shadowsEnable)
	{
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto& light = m_lights[i];
			if (light.castShadow && light.type == LtSpot)
				lightAtlasIndices.push_back(i);
		}
	}

	// Write all lights to sbuffer; without shadow map information.
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

	// If shadow casting directional light found add cascade shadow map pass
	// and update light sbuffer.
	if (lightCascadeIndex >= 0)
	{
		m_renderGraph->addPass(
			L"Shadow cascade",
			[&](render::RenderPassBuilder& builder)
			{
				render::Clear clear;
				clear.mask = render::CfDepth;
				clear.depth = 1.0f;
				builder.setOutput(s_handleShadowMapCascade, clear);
			},
			[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
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
						slice * 1024,
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

					WorldRenderPassForward shadowPass(
						s_techniqueShadow,
						sharedParams,
						IWorldRenderPass::PfNone,
						shadowRenderView.getView(),
						nullptr,
						nullptr,
						nullptr
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

					// Write slice coordinates to shaders.
					Vector4(
						0.0f,
						float(slice) / shadowSettings.cascadingSlices,
						1.0f,
						1.0f / shadowSettings.cascadingSlices
					).storeUnaligned(lsd->atlasTransform);
				}
			}
		);
	}

	if (!lightAtlasIndices.empty())
	{
		int32_t atlasIndex = 0;
		for (int32_t lightAtlasIndex : lightAtlasIndices)
		{
			m_renderGraph->addPass(
				L"Shadow atlas",
				[&](render::RenderPassBuilder& builder)
				{
					render::Clear clear;
					clear.mask = render::CfDepth;
					clear.depth = 1.0f;
					builder.setOutput(s_handleShadowMapAtlas, clear);
				},
				[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
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

					WorldRenderPassForward shadowPass(
						s_techniqueShadow,
						sharedParams,
						IWorldRenderPass::PfNone,
						shadowRenderView.getView(),
						nullptr,
						nullptr,
						nullptr
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
			++atlasIndex;
		}
	}
}

void WorldRendererForward::buildVisual(const WorldRenderView& worldRenderView, int32_t frame)
{
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	int32_t lightCount = (int32_t)m_lights.size();

	m_renderGraph->addPass(
		L"Visual",
		[&](render::RenderPassBuilder& builder)
		{
			builder.addInput(s_handleGBuffer);
			builder.addInput(s_handleAmbientOcclusion);

			if (shadowsEnable)
			{
				builder.addInput(s_handleShadowMapCascade);
				builder.addInput(s_handleShadowMapAtlas);
			}

			render::Clear clear;
			clear.mask = render::CfColor | render::CfDepth;
			clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
			clear.depth = 1.0f;
			builder.setOutput(s_handleVisual[0], clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);
			auto ambientOcclusionTargetSet = m_renderGraph->getRenderTarget(s_handleAmbientOcclusion);
			auto shadowCascadeTargetSet = m_renderGraph->getRenderTarget(s_handleShadowMapCascade);
			auto shadowAtlasTargetSet = m_renderGraph->getRenderTarget(s_handleShadowMapAtlas);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(wc.getRenderContext());
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->endParameters(wc.getRenderContext());

			WorldRenderPassForward defaultPass(
				s_techniqueForwardColor,
				sharedParams,
				IWorldRenderPass::PfLast,
				worldRenderView.getView(),
				m_frames[frame].lightSBuffer,
				lightCount,
				m_settings.fog,
				m_settings.fogDistanceY,
				m_settings.fogDistanceZ,
				m_settings.fogDensityY,
				m_settings.fogDensityZ,
				m_settings.fogColor,
				nullptr,
				gbufferTargetSet->getColorTexture(0),
				(ambientOcclusionTargetSet != nullptr) ? ambientOcclusionTargetSet->getColorTexture(0) : nullptr,
				(shadowCascadeTargetSet != nullptr) ? shadowCascadeTargetSet->getDepthTexture() : nullptr,
				(shadowAtlasTargetSet != nullptr) ? shadowAtlasTargetSet->getDepthTexture() : nullptr
			);

			T_ASSERT(!wc.getRenderContext()->havePendingDraws());
			wc.build(worldRenderView, defaultPass, m_rootEntity);
			wc.flush(worldRenderView, defaultPass);
			wc.getRenderContext()->merge(render::RpAll);
		}
	);
}

void WorldRendererForward::buildProcess(const WorldRenderView& worldRenderView)
{
	render::ImageProcessStep::Instance::RenderParams params;
	params.viewFrustum = worldRenderView.getViewFrustum();
	params.viewToLight = Matrix44::identity(); //f.viewToLightSpace;
	params.view = worldRenderView.getView();
	params.projection = worldRenderView.getProjection();
	params.deltaTime = 1.0f / 60.0f; // deltaTime;

	StaticVector< render::ImageProcess*, 4 > processes;
	if (m_toneMapImageProcess)
		processes.push_back(m_toneMapImageProcess);
	if (m_visualImageProcess)
		processes.push_back(m_visualImageProcess);
	if (m_gammaCorrectionImageProcess)
		processes.push_back(m_gammaCorrectionImageProcess);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);

	for (size_t i = 0; i < processes.size(); ++i)
	{
		auto process = processes[i];
		T_ASSERT(process != nullptr);

		bool haveNext = bool((i + 1) < processes.size());
		m_renderGraph->addPass(
			L"Process",
			[&](render::RenderPassBuilder& builder)
			{
				if (haveNext)
					builder.setOutput(s_handleVisual[i + 1]);

				builder.addInput(s_handleGBuffer);
				builder.addInput(s_handleVisual[i]);
			},
			[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
			{
				auto gbufferTargetSet = resources.getInput(s_handleGBuffer);
				auto visualTargetSet = resources.getInput(s_handleVisual[i]);

				render::ImageProcessStep::Instance::RenderParams params;
				params.viewFrustum = worldRenderView.getViewFrustum();
				params.view = worldRenderView.getView();
				params.projection = worldRenderView.getProjection();
				params.deltaTime = 1.0f / 60.0f; // \tbd deltaTime;

				process->build(
					renderContext,
					visualTargetSet->getColorTexture(0),	// color
					gbufferTargetSet->getColorTexture(0),	// depth
					gbufferTargetSet->getColorTexture(1),	// normal
					nullptr,	// velocity
					nullptr,	// shadow mask
					params
				);
			}
		);
	}
}

	}
}
