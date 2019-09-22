#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Random.h"
#include "Core/Math/Float.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/RenderTargetSet.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Render/ImageProcess/ImageProcessTargetPool.h"
#include "Resource/Id.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Entity/GroupEntity.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
#include "World/SMProj/TrapezoidShadowProjection.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const int32_t c_maxLightCount = 1024;

const resource::Id< render::ImageProcessSettings > c_ambientOcclusionLow(Guid(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}"));		// SSAO, half size
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionMedium(Guid(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}"));	// SSAO, full size
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionHigh(Guid(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}"));		// HBAO, half size
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionUltra(Guid(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}"));	// HBAO, full size
const resource::Id< render::ImageProcessSettings > c_antiAliasNone(Guid(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasLow(Guid(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasMedium(Guid(L"{3E1D810B-339A-F742-9345-4ECA00220D57}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasHigh(Guid(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasUltra(Guid(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}"));
const resource::Id< render::ImageProcessSettings > c_gammaCorrection(Guid(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}"));
const resource::Id< render::ImageProcessSettings > c_toneMap(Guid(L"{BC4FA128-A976-4023-A422-637581ADFD7E}"));

render::handle_t s_techniqueForwardColor = 0;
render::handle_t s_techniqueForwardGBufferWrite = 0;
render::handle_t s_techniqueShadow = 0;
render::handle_t s_handleTime = 0;
render::handle_t s_handleView = 0;
render::handle_t s_handleViewInverse = 0;
render::handle_t s_handleProjection = 0;

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererForward", 0, WorldRendererForward, IWorldRenderer)

WorldRendererForward::WorldRendererForward()
:	m_toneMapQuality(QuDisabled)
,	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
,	m_count(0)
{
	// Techniques
	s_techniqueForwardColor = render::getParameterHandle(L"World_ForwardColor");
	s_techniqueForwardGBufferWrite = render::getParameterHandle(L"World_ForwardGBufferWrite");
	s_techniqueShadow = render::getParameterHandle(L"World_ShadowWrite");

	// Global parameters.
	s_handleTime = render::getParameterHandle(L"World_Time");
	s_handleView = render::getParameterHandle(L"World_View");
	s_handleViewInverse = render::getParameterHandle(L"World_ViewInverse");
	s_handleProjection = render::getParameterHandle(L"World_Projection");
}

bool WorldRendererForward::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.toneMapQuality;
	m_shadowsQuality = desc.shadowsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;

	// Allocate frames, one for each queued frame.
	m_frames.resize(desc.frameCount);

	// Create post process target pool to enable sharing of targets between multiple processes.
	Ref< render::ImageProcessTargetPool > postProcessTargetPool = new render::ImageProcessTargetPool(renderSystem);

	// Create "mini gbuffer" targets.
	{
		render::RenderTargetSetCreateDesc rtscd;

		rtscd.count = 3;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.targets[0].format = render::TfR16F;		// Depth (R)
		rtscd.targets[1].format = render::TfR16G16F;	// Normals (RG)
		rtscd.targets[2].format = render::TfR8;			// Ambient occlusion (R)

		m_gbufferTargetSet = renderSystem->createRenderTargetSet(rtscd);
		if (!m_gbufferTargetSet)
		{
			log::error << L"Unable to create depth render target." << Endl;
			return false;
		}
	}

	// Allocate "shadow map" targets.
	if (m_shadowsQuality > QuDisabled)
	{
		// \tbd Do we need different projections? Not used...
		m_shadowProjection = new UniformShadowProjection(1024);

		// Create shadow cascade map target.
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 0;
		rtscd.width = 1024;
		rtscd.height = m_settings.shadowSettings[m_shadowsQuality].cascadingSlices * 1024;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		if ((m_shadowCascadeTargetSet = renderSystem->createRenderTargetSet(rtscd)) == nullptr)
			m_shadowsQuality = QuDisabled;

		// Create shadow atlas map target.
		rtscd.count = 0;
		rtscd.width =
		rtscd.height = 4096;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		if ((m_shadowAtlasTargetSet = renderSystem->createRenderTargetSet(rtscd)) == nullptr)
			m_shadowsQuality = QuDisabled;

		// Ensure targets are destroyed if something went wrong in setup.
		if (m_shadowsQuality == QuDisabled)
		{
			safeDestroy(m_shadowCascadeTargetSet);
			safeDestroy(m_shadowAtlasTargetSet);
		}
	}

	// Create ambient occlusion processing.
	{
		resource::Id< render::ImageProcessSettings > ambientOcclusionId;
		resource::Proxy< render::ImageProcessSettings > ambientOcclusion;

		switch (m_ambientOcclusionQuality)
		{
		default:
		case QuDisabled:
			break;

		case QuLow:
			ambientOcclusionId = c_ambientOcclusionLow;
			break;

		case QuMedium:
			ambientOcclusionId = c_ambientOcclusionMedium;
			break;

		case QuHigh:
			ambientOcclusionId = c_ambientOcclusionHigh;
			break;

		case QuUltra:
			ambientOcclusionId = c_ambientOcclusionUltra;
			break;
		}

		if (ambientOcclusionId)
		{
			if (!resourceManager->bind(ambientOcclusionId, ambientOcclusion))
				log::warning << L"Unable to create ambient occlusion process; AO disabled" << Endl;
		}

		if (ambientOcclusion)
		{
			m_ambientOcclusion = new render::ImageProcess();
			if (!m_ambientOcclusion->create(
				ambientOcclusion,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create ambient occlusion process; AO disabled" << Endl;
				m_ambientOcclusion = nullptr;
			}
		}
	}

	// Create antialias processing.
	{
		resource::Id< render::ImageProcessSettings > antiAliasId;
		resource::Proxy< render::ImageProcessSettings > antiAlias;

		switch (m_antiAliasQuality)
		{
		default:
		case QuDisabled:
			break;

		case QuLow:
			antiAliasId = c_antiAliasLow;
			break;

		case QuMedium:
			antiAliasId = c_antiAliasMedium;
			break;

		case QuHigh:
			antiAliasId = c_antiAliasHigh;
			break;

		case QuUltra:
			antiAliasId = c_antiAliasUltra;
			break;
		}

		if (antiAliasId)
		{
			if (!resourceManager->bind(antiAliasId, antiAlias))
				log::warning << L"Unable to create antialias process; AA disabled" << Endl;
		}

		if (antiAlias)
		{
			m_antiAlias = new render::ImageProcess();
			if (!m_antiAlias->create(
				antiAlias,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create antialias process; AA disabled" << Endl;
				m_antiAlias = nullptr;
			}
		}
	}

	// Create "visual" post processing filter.
	{
		const resource::Id< render::ImageProcessSettings >& imageProcessSettings = desc.worldRenderSettings->imageProcess[desc.imageProcessQuality];
		if (imageProcessSettings)
		{
			resource::Proxy< render::ImageProcessSettings > imageProcess;
			if (!resourceManager->bind(imageProcessSettings, imageProcess))
				log::warning << L"Unable to create visual post processing image filter; post processing disabled" << Endl;

			if (imageProcess)
			{
				m_visualImageProcess = new render::ImageProcess();
				if (!m_visualImageProcess->create(
					imageProcess,
					postProcessTargetPool,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height,
					desc.allTargetsPersistent
				))
				{
					log::warning << L"Unable to create visual post processing; post processing disabled" << Endl;
					m_visualImageProcess = nullptr;
				}
			}
		}
	}

	// Create gamma correction processing.
	if (m_settings.linearLighting)
	{
		resource::Proxy< render::ImageProcessSettings > gammaCorrection;
		if (!resourceManager->bind(c_gammaCorrection, gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled" << Endl;

		if (gammaCorrection)
		{
			m_gammaCorrectionImageProcess = new render::ImageProcess();
			if (m_gammaCorrectionImageProcess->create(
				gammaCorrection,
				postProcessTargetPool,
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
				log::warning << L"Unable to create gamma correction process; gamma correction disabled" << Endl;
				m_gammaCorrectionImageProcess = nullptr;
			}
		}
	}

	// Create tone map processing.
	if (m_toneMapQuality > QuDisabled)
	{
		resource::Proxy< render::ImageProcessSettings > toneMap;

		if (!resourceManager->bind(c_toneMap, toneMap))
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = QuDisabled;
		}

		if (toneMap)
		{
			m_toneMapImageProcess = new render::ImageProcess();
			if (m_toneMapImageProcess->create(
				toneMap,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
				m_toneMapImageProcess->setFloatParameter(
					render::getParameterHandle(L"World_ExposureBias"),
					m_settings.exposureBias
				);
			else
			{
				log::warning << L"Unable to create tone map process; tone mapping disabled." << Endl;
				m_toneMapImageProcess = nullptr;
				m_toneMapQuality = QuDisabled;
			}
		}
	}

	// Create "visual" and "intermediate" target.
	if (m_antiAlias || m_visualImageProcess || m_gammaCorrectionImageProcess)
	{
		render::RenderTargetSetCreateDesc rtscd;

		rtscd.count = 1;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;

		if (m_settings.linearLighting || (m_visualImageProcess && m_visualImageProcess->requireHighRange()))
			rtscd.targets[0].format = render::TfR11G11B10F;
		else
			rtscd.targets[0].format = render::TfR8G8B8A8;

		m_visualTargetSet = renderSystem->createRenderTargetSet(rtscd);
		if (!m_visualTargetSet)
			return false;

		m_intermediateTargetSet = renderSystem->createRenderTargetSet(rtscd);
		if (!m_intermediateTargetSet)
			return false;
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

	// Allocate "depth" context.
	for (auto& frame : m_frames)
		frame.depth = new WorldContext(desc.entityRenderers);

	// Allocate "shadow" contexts for each slice.
	if (m_shadowsQuality > QuDisabled)
	{
		for (auto& frame : m_frames)
		{
			for (int32_t i = 0; i < m_settings.shadowSettings[m_shadowsQuality].cascadingSlices; ++i)
				frame.slice[i].shadow = new WorldContext(desc.entityRenderers);

			for (int32_t i = 0; i < 16; ++i)
				frame.atlas[i].shadow = new WorldContext(desc.entityRenderers);
		}
	}

	// Allocate "visual" contexts.
	for (auto& frame : m_frames)
		frame.visual = new WorldContext(desc.entityRenderers);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(16 * 1024);

	// Determine slice distances.
	const auto& shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / shadowSettings.cascadingSlices;
		float log = powf(ii, shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
	}
	m_slicePositions[shadowSettings.cascadingSlices] = shadowSettings.farZ;

	m_rootEntity = new GroupEntity();
	m_count = 0;
	return true;
}

void WorldRendererForward::destroy()
{
	for (auto& frame : m_frames)
	{
		for (int32_t i = 0; i < MaxSliceCount; ++i)
			frame.slice[i].shadow = nullptr;

		for (int32_t i = 0; i < 16; ++i)
			frame.atlas[i].shadow = nullptr;

		frame.visual = nullptr;
		frame.depth = nullptr;

		safeDestroy(frame.lightSBuffer);
	}

	safeDestroy(m_toneMapImageProcess);
	safeDestroy(m_gammaCorrectionImageProcess);
	safeDestroy(m_visualImageProcess);
	safeDestroy(m_shadowCascadeTargetSet);
	safeDestroy(m_shadowAtlasTargetSet);
	safeDestroy(m_gbufferTargetSet);
	safeDestroy(m_visualTargetSet);
}

void WorldRendererForward::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererForward::build(WorldRenderView& worldRenderView, int32_t frame)
{
	Frame& f = m_frames[frame];

	// Flush render contexts.
	if (f.haveDepth)
	{
		f.depth->getRenderContext()->flush();
		f.haveDepth = false;
	}

	if (f.haveShadows)
	{
		int32_t cascadingSlices = m_settings.shadowSettings[m_shadowsQuality].cascadingSlices;
		for (int32_t i = 0; i < cascadingSlices; ++i)
			f.slice[i].shadow->getRenderContext()->flush();

		for (int32_t i = 0; i < 16; ++i)
			f.atlas[i].shadow->getRenderContext()->flush();
	}

	f.visual->getRenderContext()->flush();

	// Begun building new frame.
	const Matrix44& view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();

	worldRenderView.setEyePosition(viewInverse.translation().xyz1());
	worldRenderView.setEyeDirection(viewInverse.axisZ().xyz0());

	// Store some global values.
	f.time = worldRenderView.getTime();

	// \tbd Improve light iteration... 
	worldRenderView.resetLights();

	// Build depth context.
	buildGBuffer(worldRenderView, frame);

	// \tbd Lights have now been collected as a side effect...

	// Build light and shadow contexts.
	buildLights(worldRenderView, frame);

	// \tbd Tiled light culling.

	// Build visual context.
	buildVisual(worldRenderView, frame);

	m_count++;
	m_rootEntity->removeAllEntities();
}

bool WorldRendererForward::beginRender(render::IRenderView* renderView, int32_t frame, const Color4f& clearColor)
{
	// If we don't have a visual target set then we cannot clear.
	if (!m_visualTargetSet)
		return true;

	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth;
	clear.colors[0] = clearColor;
	clear.depth = 1.0f;

	if (!renderView->begin(m_visualTargetSet, 0, &clear))
		return false;

	return true;
}

void WorldRendererForward::render(render::IRenderView* renderView, int32_t frame)
{
	Frame& f = m_frames[frame];
	render::Clear clear;

	// Prepare global program parameters.
	render::ProgramParameters defaultProgramParams;
	defaultProgramParams.beginParameters(m_globalContext);
	defaultProgramParams.setFloatParameter(s_handleTime, f.time);
	defaultProgramParams.setMatrixParameter(s_handleProjection, f.projection);
	defaultProgramParams.endParameters(m_globalContext);

	// Render gbuffer.
	{
		T_RENDER_PUSH_MARKER(renderView, "World: GBuffer");

		const float clearZ = f.viewFrustum.getFarZ();

		clear.mask = render::CfColor | render::CfDepth;
		clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);	// depth
		clear.colors[1] = Color4f(0.0f, 0.0f, 1.0f, 0.0f);	// normal
		clear.colors[2] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);	// ao
		clear.depth = 1.0f;

		if (renderView->begin(m_gbufferTargetSet, &clear))
		{
			f.depth->getRenderContext()->render(renderView, render::RpOpaque, &defaultProgramParams);
			renderView->end();
		}
		T_RENDER_POP_MARKER(renderView);
	}

	// Render gbuffer ambient occlusion.
	if (m_ambientOcclusion)
	{
		T_RENDER_PUSH_MARKER(renderView, "World: GBuffer AO");
		if (renderView->begin(m_gbufferTargetSet, 2, nullptr))
		{
			render::ImageProcessStep::Instance::RenderParams params;
			params.viewFrustum = f.viewFrustum;
			params.view = f.view;
			params.projection = f.projection;
			params.deltaTime = 0.0f;

			m_ambientOcclusion->render(
				renderView,
				nullptr,	// color
				m_gbufferTargetSet->getColorTexture(0),	// depth
				m_gbufferTargetSet->getColorTexture(1),	// normal
				nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
			renderView->end();
		}
		T_RENDER_POP_MARKER(renderView);
	}

	// Render shadow map.
	if (f.haveShadows)
	{
		// Shadow atlas.
		T_RENDER_PUSH_MARKER(renderView, "World: Shadow map, atlas");

		clear.mask = render::CfDepth;
		clear.depth = 1.0f;

		if (renderView->begin(m_shadowAtlasTargetSet, &clear))
		{
			for (int32_t i = 0; i < 16; ++i)
			{
				render::ProgramParameters shadowProgramParams;
				shadowProgramParams.beginParameters(m_globalContext);
				shadowProgramParams.setFloatParameter(s_handleTime, f.time);
				shadowProgramParams.setMatrixParameter(s_handleView, f.atlas[i].shadowLightView);
				shadowProgramParams.setMatrixParameter(s_handleViewInverse, f.atlas[i].shadowLightView.inverse());
				shadowProgramParams.setMatrixParameter(s_handleProjection, f.atlas[i].shadowLightProjection);
				shadowProgramParams.endParameters(m_globalContext);

				f.atlas[i].shadow->getRenderContext()->render(renderView, render::RpSetup | render::RpOpaque, &shadowProgramParams);
			}
			renderView->end();
		}
		T_RENDER_POP_MARKER(renderView);

		// Directional shadow cascades.
		T_RENDER_PUSH_MARKER(renderView, "World: Shadow map, cascades");

		clear.mask = render::CfDepth;
		clear.depth = 1.0f;

		if (renderView->begin(m_shadowCascadeTargetSet, &clear))
		{
			for (int32_t i = 0; i < m_settings.shadowSettings[m_shadowsQuality].cascadingSlices; ++i)
			{
				render::ProgramParameters shadowProgramParams;
				shadowProgramParams.beginParameters(m_globalContext);
				shadowProgramParams.setFloatParameter(s_handleTime, f.time);
				shadowProgramParams.setMatrixParameter(s_handleView, f.slice[i].shadowLightView);
				shadowProgramParams.setMatrixParameter(s_handleViewInverse, f.slice[i].shadowLightView.inverse());
				shadowProgramParams.setMatrixParameter(s_handleProjection, f.slice[i].shadowLightProjection);
				shadowProgramParams.endParameters(m_globalContext);

				f.slice[i].shadow->getRenderContext()->render(renderView, render::RpSetup | render::RpOpaque, &shadowProgramParams);
			}
			renderView->end();
		}
		T_RENDER_POP_MARKER(renderView);
	}

	// Render visuals.
	{
		T_RENDER_PUSH_MARKER(renderView, "World: Visual");
		f.visual->getRenderContext()->render(renderView, render::RpAll, &defaultProgramParams);
		T_RENDER_POP_MARKER(renderView);
	}

	m_globalContext->flush();
}

void WorldRendererForward::endRender(render::IRenderView* renderView, int32_t frame, float deltaTime)
{
	Frame& f = m_frames[frame];

	if (m_visualTargetSet)
	{
		renderView->end();

		render::ImageProcessStep::Instance::RenderParams params;
		params.viewFrustum = f.viewFrustum;
		params.view = f.view;
		params.projection = f.projection;
		params.deltaTime = deltaTime;

		render::RenderTargetSet* sourceTargetSet = m_visualTargetSet;
		render::RenderTargetSet* outputTargetSet = m_intermediateTargetSet;
		T_ASSERT(sourceTargetSet);

		StaticVector< render::ImageProcess*, 5 > processes;
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
			T_RENDER_PUSH_MARKER(renderView, "World: Post process");

			bool haveNext = bool((i + 1) < processes.size());
			if (haveNext)
				renderView->begin(outputTargetSet, nullptr);

			processes[i]->render(
				renderView,
				sourceTargetSet->getColorTexture(0),	// color
				m_gbufferTargetSet->getColorTexture(0),	// depth
				m_gbufferTargetSet->getColorTexture(1),	// normal
				nullptr,	// velocity
				m_shadowCascadeTargetSet ? m_shadowCascadeTargetSet->getColorTexture(0) : nullptr,	// shadow mask
				params
			);

			if (haveNext)
			{
				renderView->end();
				std::swap(sourceTargetSet, outputTargetSet);
			}

			T_RENDER_POP_MARKER(renderView);
		}
	}
}

render::ImageProcess* WorldRendererForward::getVisualImageProcess()
{
	return m_visualImageProcess;
}

void WorldRendererForward::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
	if (m_visualTargetSet)
		outTargets.push_back(render::DebugTarget(L"Visual", render::DtvDefault, m_visualTargetSet->getColorTexture(0)));

	if (m_intermediateTargetSet)
		outTargets.push_back(render::DebugTarget(L"Intermediate", render::DtvDefault, m_intermediateTargetSet->getColorTexture(0)));

	if (m_gbufferTargetSet)
	{
		outTargets.push_back(render::DebugTarget(L"GBuffer depth", render::DtvViewDepth, m_gbufferTargetSet->getColorTexture(0)));
		outTargets.push_back(render::DebugTarget(L"GBuffer normals", render::DtvNormals, m_gbufferTargetSet->getColorTexture(1)));
		outTargets.push_back(render::DebugTarget(L"GBuffer AO", render::DtvDefault, m_gbufferTargetSet->getColorTexture(2)));
	}

	if (m_shadowCascadeTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow map (cascade)", render::DtvShadowMap, m_shadowCascadeTargetSet->getDepthTexture()));

	if (m_shadowAtlasTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow map (atlas)", render::DtvShadowMap, m_shadowAtlasTargetSet->getDepthTexture()));

	if (m_ambientOcclusion)
		m_ambientOcclusion->getDebugTargets(outTargets);

	if (m_antiAlias)
		m_antiAlias->getDebugTargets(outTargets);

	if (m_visualImageProcess)
		m_visualImageProcess->getDebugTargets(outTargets);

	if (m_gammaCorrectionImageProcess)
		m_gammaCorrectionImageProcess->getDebugTargets(outTargets);

	if (m_toneMapImageProcess)
		m_toneMapImageProcess->getDebugTargets(outTargets);
}

void WorldRendererForward::buildGBuffer(WorldRenderView& worldRenderView, int32_t frame)
{
	Frame& f = m_frames[frame];

	WorldRenderPassForward pass(
		s_techniqueForwardGBufferWrite,
		IWorldRenderPass::PfFirst,
		worldRenderView.getView(),
		nullptr,
		nullptr,
		nullptr
	);
	f.depth->build(worldRenderView, pass, m_rootEntity);
	f.depth->flush(worldRenderView, pass, m_rootEntity);

	f.haveDepth = true;
}

void WorldRendererForward::buildLights(WorldRenderView& worldRenderView, int32_t frame)
{
	Frame& f = m_frames[frame];

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);

	LightShaderData* lightShaderData = (LightShaderData*)f.lightSBuffer->lock();
	T_FATAL_ASSERT(lightShaderData != nullptr);

	int32_t atlasIndex = 0;
	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);

		lightShaderData->typeRangeRadius[0] = (float)light.type;
		lightShaderData->typeRangeRadius[1] = light.range;
		lightShaderData->typeRangeRadius[2] = light.radius / 2.0f;
		lightShaderData->typeRangeRadius[3] = 0.0f;
		
		(view * light.position.xyz1()).storeUnaligned(lightShaderData->position);
		(view * light.direction.xyz0()).storeUnaligned(lightShaderData->direction);
		light.color.storeUnaligned(lightShaderData->color);

		if (shadowsEnable && light.castShadow && light.type == LtDirectional)
		{
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;
			Frustum shadowFrustum;

			const auto& shadowSettings = m_settings.shadowSettings[m_shadowsQuality];

			for (int32_t slice = 0; slice < shadowSettings.cascadingSlices; ++slice)
			{
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], shadowSettings.farZ));

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

				// Calculate shadow map projection.
				m_shadowProjection->calculate(
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

				WorldRenderView shadowRenderView;
				shadowRenderView.resetLights();
				shadowRenderView.setProjection(shadowLightProjection);
				shadowRenderView.setView(shadowLightView, shadowLightView);
				shadowRenderView.setViewFrustum(shadowFrustum);
				shadowRenderView.setCullFrustum(shadowFrustum);
				shadowRenderView.setEyePosition(worldRenderView.getEyePosition());
				shadowRenderView.setTimes(
					worldRenderView.getTime(),
					worldRenderView.getDeltaTime(),
					worldRenderView.getInterval()
				);

				WorldRenderPassForward shadowPass(
					s_techniqueShadow,
					IWorldRenderPass::PfNone,
					shadowRenderView.getView(),
					nullptr,
					nullptr,
					nullptr
				);

				// Set viewport to current cascade.
				auto svrb = f.slice[slice].shadow->getRenderContext()->alloc< render::SetViewportRenderBlock >();
				svrb->viewport = render::Viewport(
					0,
					slice * 1024,
					1024,
					1024,
					0.0f,
					1.0f
				);
				f.slice[slice].shadow->getRenderContext()->draw(render::RpSetup, svrb);	

				f.slice[slice].shadow->build(shadowRenderView, shadowPass, m_rootEntity);
				f.slice[slice].shadow->flush(shadowRenderView, shadowPass, m_rootEntity);

				f.slice[slice].shadowLightView = shadowLightView;
				f.slice[slice].shadowLightProjection = shadowLightProjection;
				f.slice[slice].viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;

				// Write transposed matrix to shaders as shaders have row-major order.
				Matrix44 vls = f.slice[slice].viewToLightSpace.transpose();
				vls.axisX().storeUnaligned(lightShaderData->viewToLight0);
				vls.axisY().storeUnaligned(lightShaderData->viewToLight1);
				vls.axisZ().storeUnaligned(lightShaderData->viewToLight2);
				vls.translation().storeUnaligned(lightShaderData->viewToLight3);

				// Write slice coordinates to shaders.
				Vector4(
					0.0f,
					float(slice) / shadowSettings.cascadingSlices,
					1.0f,
					1.0f / shadowSettings.cascadingSlices
				).storeUnaligned(lightShaderData->atlasTransform);
			}
		}
		else if (shadowsEnable && light.castShadow && light.type == LtSpot)
		{
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

			WorldRenderView shadowRenderView;
			shadowRenderView.resetLights();
			shadowRenderView.setProjection(shadowLightProjection);
			shadowRenderView.setView(shadowLightView, shadowLightView);
			shadowRenderView.setViewFrustum(shadowFrustum);
			shadowRenderView.setCullFrustum(shadowFrustum);
			shadowRenderView.setEyePosition(worldRenderView.getEyePosition());
			shadowRenderView.setTimes(
				worldRenderView.getTime(),
				worldRenderView.getDeltaTime(),
				worldRenderView.getInterval()
			);

			WorldRenderPassForward shadowPass(
				s_techniqueShadow,
				IWorldRenderPass::PfNone,
				shadowRenderView.getView(),
				nullptr,
				nullptr,
				nullptr
			);

			// Set viewport to light atlas slot.
			auto svrb = f.atlas[atlasIndex].shadow->getRenderContext()->alloc< render::SetViewportRenderBlock >();
			svrb->viewport = render::Viewport(
				(atlasIndex & 3) * 1024,
				(atlasIndex / 4) * 1024,
				1024,
				1024,
				0.0f,
				1.0f
			);
			f.atlas[atlasIndex].shadow->getRenderContext()->draw(render::RpSetup, svrb);	

			f.atlas[atlasIndex].shadow->build(shadowRenderView, shadowPass, m_rootEntity);
			f.atlas[atlasIndex].shadow->flush(shadowRenderView, shadowPass, m_rootEntity);

			f.atlas[atlasIndex].shadowLightView = shadowLightView;
			f.atlas[atlasIndex].shadowLightProjection = shadowLightProjection;
			f.atlas[atlasIndex].viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;

			// Write transposed matrix to shaders as shaders have row-major order.
			Matrix44 vls = f.atlas[atlasIndex].viewToLightSpace.transpose();
			vls.axisX().storeUnaligned(lightShaderData->viewToLight0);
			vls.axisY().storeUnaligned(lightShaderData->viewToLight1);
			vls.axisZ().storeUnaligned(lightShaderData->viewToLight2);
			vls.translation().storeUnaligned(lightShaderData->viewToLight3);

			// Write atlas coordinates to shaders.
			Vector4(
				(atlasIndex & 3) / 4.0f,
				(atlasIndex / 4) / 4.0f,
				1.0f / 4.0f,
				1.0f / 4.0f
			).storeUnaligned(lightShaderData->atlasTransform);

			++atlasIndex;
		}
		else if (!shadowsEnable || !light.castShadow)
		{
			Vector4::zero().storeUnaligned(lightShaderData->viewToLight0);
			Vector4::zero().storeUnaligned(lightShaderData->viewToLight1);
			Vector4::zero().storeUnaligned(lightShaderData->viewToLight2);
			Vector4::zero().storeUnaligned(lightShaderData->viewToLight3);
		}

		++lightShaderData;
	}

	f.lightSBuffer->unlock();
	f.lightCount = worldRenderView.getLightCount();
	f.haveShadows = shadowsEnable;

	worldRenderView.resetLights();
}

void WorldRendererForward::buildVisual(WorldRenderView& worldRenderView, int32_t frame)
{
	Frame& f = m_frames[frame];

	bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);

	WorldRenderPassForward defaultPass(
		s_techniqueForwardColor,
		IWorldRenderPass::PfLast,
		worldRenderView.getView(),
		f.lightSBuffer,
		f.lightCount,
		m_settings.fog,
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ,
		m_settings.fogColor,
		nullptr,
		m_gbufferTargetSet->getColorTexture(0),
		m_gbufferTargetSet->getColorTexture(2),
		shadowsEnable ? m_shadowCascadeTargetSet->getDepthTexture() : nullptr,
		shadowsEnable ? m_shadowAtlasTargetSet->getDepthTexture() : nullptr
	);
	f.visual->build(worldRenderView, defaultPass, m_rootEntity);
	f.visual->flush(worldRenderView, defaultPass, m_rootEntity);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
}

	}
}
