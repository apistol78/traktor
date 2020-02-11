#include "Core/Log/Log.h"
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

const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}") ; // L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}");
const resource::Id< render::ImageGraph > c_antiAliasLow(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}");
const resource::Id< render::ImageGraph > c_antiAliasMedium(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{3E1D810B-339A-F742-9345-4ECA00220D57}");
const resource::Id< render::ImageGraph > c_antiAliasHigh(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{0C288028-7BFD-BE46-A25F-F3910BE50319}");
const resource::Id< render::ImageGraph > c_antiAliasUltra(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}") ; // L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}");
const resource::Id< render::ImageGraph > c_gammaCorrection(L"{B1E8367D-91DD-D648-A44F-B86492169771}") ; // L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}");
const resource::Id< render::ImageGraph > c_toneMapFixed(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}") ; // L"{838922A0-49CE-6645-8A9C-BA0E71081033}");
const resource::Id< render::ImageGraph > c_toneMapAdaptive(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}") ; // L"{BC4FA128-A976-4023-A422-637581ADFD7E}");

// Techniques
const render::Handle s_techniqueForwardColor(L"World_ForwardColor");
const render::Handle s_techniqueForwardGBufferWrite(L"World_ForwardGBufferWrite");
const render::Handle s_techniqueShadow(L"World_ShadowWrite");

// Global parameters.
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleProjection(L"World_Projection");
const render::Handle s_handleGamma(L"World_Gamma");
const render::Handle s_handleGammaInverse(L"World_GammaInverse");
const render::Handle s_handleExposure(L"World_Exposure");

// RenderGraph targets.
const render::Handle s_handleGBuffer(L"GBuffer");
const render::Handle s_handleAmbientOcclusion(L"AmbientOcclusion");
const render::Handle s_handleShadowMapCascade(L"ShadowMapCascade");
const render::Handle s_handleShadowMapAtlas(L"ShadowMapAtlas");
const render::Handle s_handleVisual[6] =
{
	render::Handle(L"Visual0"),
	render::Handle(L"Visual1"),
	render::Handle(L"Visual2"),
	render::Handle(L"Visual3"),
	render::Handle(L"Visual4"),
	render::Handle(L"Visual5")
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
	render::RenderGraphTargetSetDesc rgtd;
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
	rgtd = render::RenderGraphTargetSetDesc();
	rgtd.count = 2;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR16F;		// Depth (R)
	rgtd.targets[1].colorFormat = render::TfR16G16F;	// Normals (RG)
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
	 m_renderGraph->addTargetSet(s_handleGBuffer, rgtd, desc.sharedDepthStencil);

	// Ambient occlusion.
	rgtd = render::RenderGraphTargetSetDesc();
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	rgtd.screenWidthDenom = 1;
	rgtd.screenHeightDenom = 1;
	m_renderGraph->addTargetSet(s_handleAmbientOcclusion, rgtd, desc.sharedDepthStencil);

	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	if (shadowsEnable)
	{
		const auto& shadowSettings = m_settings.shadowSettings[m_shadowsQuality];

		// Cascading shadow map.
		rgtd = render::RenderGraphTargetSetDesc();
		rgtd.count = 0;
		rgtd.width = shadowSettings.resolution;
		rgtd.height = shadowSettings.cascadingSlices * shadowSettings.resolution;
		rgtd.createDepthStencil = true;
		rgtd.usingPrimaryDepthStencil = false;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		m_renderGraph->addTargetSet(s_handleShadowMapCascade, rgtd);

		// Atlas shadow map.
		rgtd = render::RenderGraphTargetSetDesc();
		rgtd.count = 0;
		rgtd.width =
		rgtd.height = 4096;
		rgtd.createDepthStencil = true;
		rgtd.usingPrimaryDepthStencil = false;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		m_renderGraph->addTargetSet(s_handleShadowMapAtlas, rgtd);
	}

	// Visual
	for (int32_t i = 0; i < sizeof_array(s_handleVisual); ++i)
	{
		rgtd = render::RenderGraphTargetSetDesc();
		rgtd.count = 1;
		rgtd.createDepthStencil = false;
		rgtd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rgtd.targets[0].colorFormat = render::TfR11G11B10F;
		rgtd.screenWidthDenom = 1;
		rgtd.screenHeightDenom = 1;
		m_renderGraph->addTargetSet(s_handleVisual[i], rgtd, desc.sharedDepthStencil);
	}

	// Allocate render contexts.
	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(1 * 1024 * 1024);

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Add target sets from image graphs.
	if (m_ambientOcclusion)
		m_ambientOcclusion->addTargetSets(m_renderGraph);
	if (m_antiAlias)
		m_antiAlias->addTargetSets(m_renderGraph);
	if (m_visual)
		m_visual->addTargetSets(m_renderGraph);
	if (m_gammaCorrection)
		m_gammaCorrection->addTargetSets(m_renderGraph);
	if (m_toneMap)
		m_toneMap->addTargetSets(m_renderGraph);

	m_count = 0;
	return true;
}

void WorldRendererForward::destroy()
{
	for (auto& frame : m_frames)
		safeDestroy(frame.lightSBuffer);
	m_frames.clear();

	safeDestroy(m_renderGraph);
	safeDestroy(m_screenRenderer);
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

void WorldRendererForward::buildGBuffer(const WorldRenderView& worldRenderView)
{
	const float clearZ = m_settings.viewFarZ;

	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");
	
	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth;
	clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);	// depth
	clear.colors[1] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);	// normal
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

	m_renderGraph->addPass(rp);
}

void WorldRendererForward::buildAmbientOcclusion(const WorldRenderView& worldRenderView)
{
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

		m_renderGraph->addPass(rp);
	}

	if (!lightAtlasIndices.empty())
	{
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

			m_renderGraph->addPass(rp);

			++atlasIndex;
		}
	}
}

void WorldRendererForward::buildVisual(const WorldRenderView& worldRenderView, int32_t frame)
{
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	int32_t lightCount = (int32_t)m_lights.size();

	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(s_handleGBuffer);
	rp->addInput(s_handleAmbientOcclusion);

	if (shadowsEnable)
	{
		rp->addInput(s_handleShadowMapCascade);
		rp->addInput(s_handleShadowMapAtlas);
	}

	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	clear.depth = 1.0f;
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
			auto shadowCascadeTargetSet = renderGraph.getTargetSet(s_handleShadowMapCascade);
			auto shadowAtlasTargetSet = renderGraph.getTargetSet(s_handleShadowMapAtlas);

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

	m_renderGraph->addPass(rp);
}

void WorldRendererForward::buildProcess(const WorldRenderView& worldRenderView)
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
	cx.setParams(ipd);

	StaticVector< render::ImageGraph*, 4 > processes;
	if (m_toneMap)
		processes.push_back(m_toneMap);
	if (m_visual)
		processes.push_back(m_visual);
	if (m_gammaCorrection)
		processes.push_back(m_gammaCorrection);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);

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
