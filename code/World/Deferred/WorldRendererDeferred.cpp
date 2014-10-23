#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Random.h"
#include "Core/Math/Float.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/WorldContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderView.h"
#include "World/Deferred/LightRendererDeferred.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Deferred/WorldRenderPassDeferred.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessTargetPool.h"
#include "World/SMProj/BoxShadowProjection.h"
#include "World/SMProj/LiSPShadowProjection.h"
#include "World/SMProj/TrapezoidShadowProjection.h"
#include "World/SMProj/UniformShadowProjection.h"
#include "World/SwHiZ/WorldCullingSwRaster.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const resource::Id< PostProcessSettings > c_colorTargetCopy(Guid(L"{7DCC28A2-C357-B54F-ACF4-8159301B1764}"));
const resource::Id< PostProcessSettings > c_ambientOcclusionLow(Guid(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}"));		//< SSAO, half size
const resource::Id< PostProcessSettings > c_ambientOcclusionMedium(Guid(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}"));	//< SSAO, full size
const resource::Id< PostProcessSettings > c_ambientOcclusionHigh(Guid(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}"));		//< HBAO, half size
const resource::Id< PostProcessSettings > c_ambientOcclusionUltra(Guid(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}"));		//< HBAO, full size
const resource::Id< PostProcessSettings > c_antiAliasNone(Guid(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}"));
const resource::Id< PostProcessSettings > c_antiAliasLow(Guid(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}"));
const resource::Id< PostProcessSettings > c_antiAliasMedium(Guid(L"{3E1D810B-339A-F742-9345-4ECA00220D57}"));
const resource::Id< PostProcessSettings > c_antiAliasHigh(Guid(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}"));
const resource::Id< PostProcessSettings > c_antiAliasUltra(Guid(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}"));
const resource::Id< PostProcessSettings > c_gammaCorrection(Guid(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}"));

const static float c_interocularDistance = 6.5f;
const static float c_distortionValue = 0.8f;
const static float c_screenPlaneDistance = 13.0f;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, IWorldRenderer)

render::handle_t WorldRendererDeferred::ms_techniqueDeferredColor = 0;
render::handle_t WorldRendererDeferred::ms_techniqueDeferredGBufferWrite = 0;
render::handle_t WorldRendererDeferred::ms_techniqueShadow = 0;
render::handle_t WorldRendererDeferred::ms_handleTime = 0;
render::handle_t WorldRendererDeferred::ms_handleView = 0;
render::handle_t WorldRendererDeferred::ms_handleViewInverse = 0;
render::handle_t WorldRendererDeferred::ms_handleProjection = 0;
render::handle_t WorldRendererDeferred::ms_handleSquareProjection = 0;
render::handle_t WorldRendererDeferred::ms_handleColorMap = 0;
render::handle_t WorldRendererDeferred::ms_handleDepthMap = 0;
render::handle_t WorldRendererDeferred::ms_handleLightMap = 0;
render::handle_t WorldRendererDeferred::ms_handleNormalMap = 0;
render::handle_t WorldRendererDeferred::ms_handleReflectionMap = 0;
render::handle_t WorldRendererDeferred::ms_handleFogDistanceAndDensity = 0;
render::handle_t WorldRendererDeferred::ms_handleFogColor = 0;

WorldRendererDeferred::WorldRendererDeferred()
:	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
,	m_count(0)
{
	// Techniques
	ms_techniqueDeferredColor = render::getParameterHandle(L"World_DeferredColor");
	ms_techniqueDeferredGBufferWrite = render::getParameterHandle(L"World_DeferredGBufferWrite");
	ms_techniqueShadow = render::getParameterHandle(L"World_ShadowWrite");

	// Global parameters.
	ms_handleTime = render::getParameterHandle(L"Time");
	ms_handleView = render::getParameterHandle(L"View");
	ms_handleViewInverse = render::getParameterHandle(L"ViewInverse");
	ms_handleProjection = render::getParameterHandle(L"Projection");
	ms_handleSquareProjection = render::getParameterHandle(L"SquareProjection");
	ms_handleColorMap = render::getParameterHandle(L"ColorMap");
	ms_handleDepthMap = render::getParameterHandle(L"DepthMap");
	ms_handleLightMap = render::getParameterHandle(L"LightMap");
	ms_handleNormalMap = render::getParameterHandle(L"NormalMap");
	ms_handleReflectionMap = render::getParameterHandle(L"ReflectionMap");
	ms_handleFogDistanceAndDensity = render::getParameterHandle(L"FogDistanceAndDensity");
	ms_handleFogColor = render::getParameterHandle(L"FogColor");
}

bool WorldRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::IRenderView* renderView,
	const WorldCreateDesc& desc
)
{
	m_renderView = renderView;

	m_settings = *desc.worldRenderSettings;
	m_shadowSettings = m_settings.shadowSettings[desc.shadowsQuality];
	m_shadowsQuality = desc.shadowsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;
	m_frames.resize(desc.frameCount);

	// Pack fog parameters.
	m_fogDistanceAndDensity = Vector4(
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ
	);

	float fogColor[4];
	m_settings.fogColor.getRGBA32F(fogColor);
	m_fogColor = Vector4::loadUnaligned(fogColor);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create post process target pool to enable sharing of targets between multiple processes.
	Ref< PostProcessTargetPool > postProcessTargetPool = new PostProcessTargetPool(renderSystem);

	// Create "gbuffer" targets.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 3;
		desc.width = width;
		desc.height = height;
		desc.multiSample = desc.multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR16F;		// Depth (R)
		desc.targets[1].format = render::TfR8G8B8A8;	// Normals (RGB), Specular roughness (A)
		desc.targets[2].format = render::TfR11G11B10F;	// Surface color (RGB)

		m_gbufferTargetSet = renderSystem->createRenderTargetSet(desc);

		if (!m_gbufferTargetSet && desc.multiSample > 0)
		{
			desc.multiSample = 0;
			desc.createDepthStencil = true;
			desc.usingPrimaryDepthStencil = false;
			desc.ignoreStencil = true;

			m_gbufferTargetSet = renderSystem->createRenderTargetSet(desc);
			if (m_gbufferTargetSet)
				log::warning << L"MSAA depth render target unsupported; may cause poor performance" << Endl;
		}

		if (!m_gbufferTargetSet)
		{
			log::error << L"Unable to create depth render target" << Endl;
			return false;
		}
	}

	// Create "color read-back" target.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR11G11B10F;

		m_colorTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_colorTargetSet)
		{
			log::error << L"Unable to create color read-back render target" << Endl;
			return false;
		}
	}

	// Create "shadow map" targets.
	if (m_shadowsQuality > QuDisabled)
	{
		render::RenderSystemInformation info;
		renderSystem->getInformation(info);

		int32_t maxResolution = m_shadowSettings.resolution;
		if (info.dedicatedMemoryTotal < 512 * 1024 * 1024)
			maxResolution /= 2;

		int32_t resolution = min< int32_t >(nearestLog2(int32_t(max< int32_t >(width, height) * 1.9f)), maxResolution);
		T_DEBUG(L"Using shadow map resolution " << resolution);

		// Create shadow map target.
		render::RenderTargetSetCreateDesc desc;
		desc.count = 1;
		desc.width =
		desc.height = resolution;
		desc.multiSample = 0;
		desc.createDepthStencil = true;
		desc.usingPrimaryDepthStencil = false;
		desc.ignoreStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR16F;
		m_shadowTargetSet = renderSystem->createRenderTargetSet(desc);

		// Create shadow mask target.
		desc.count = 1;
		desc.width = width / m_shadowSettings.maskDenominator;
		desc.height = height / m_shadowSettings.maskDenominator;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8;
		desc.preferTiled = true;
		m_shadowMaskProjectTargetSet = renderSystem->createRenderTargetSet(desc);

		// Create filtered shadow mask target.
		desc.count = 1;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8;
		desc.preferTiled = true;
		m_shadowMaskFilterTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_shadowMaskFilterTargetSet)
			safeDestroy(m_shadowMaskProjectTargetSet);

		if (
			m_shadowTargetSet &&
			m_shadowMaskProjectTargetSet
		)
		{
			resource::Proxy< PostProcessSettings > shadowMaskProject;
			resource::Proxy< PostProcessSettings > shadowMaskFilter;

			if (
				!resourceManager->bind(m_shadowSettings.maskProject, shadowMaskProject) ||
				!resourceManager->bind(m_shadowSettings.maskFilter, shadowMaskFilter)
			)
			{
				log::warning << L"Unable to create shadow project process; shadows disabled (1)" << Endl;
				m_shadowsQuality = QuDisabled;
			}

			if (m_shadowsQuality > QuDisabled)
			{
				m_shadowMaskProject = new PostProcess();
				if (!m_shadowMaskProject->create(
					shadowMaskProject,
					postProcessTargetPool,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height
				))
				{
					log::warning << L"Unable to create shadow project process; shadows disabled" << Endl;
					m_shadowsQuality = QuDisabled;
				}

				m_shadowMaskFilter = new PostProcess();
				if (!m_shadowMaskFilter->create(
					shadowMaskFilter,
					postProcessTargetPool,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height
				))
				{
					log::warning << L"Unable to create shadow filter process; shadows disabled" << Endl;
					m_shadowsQuality = QuDisabled;
				}
			}
		}
		else
		{
			log::warning << L"Unable to create shadow render targets; shadows disabled" << Endl;
			m_shadowsQuality = QuDisabled;
		}

		if (m_shadowsQuality > QuDisabled)
		{
			switch (m_shadowSettings.projection)
			{
			case WorldRenderSettings::SpBox:
				m_shadowProjection0 = new BoxShadowProjection();
				break;

			case WorldRenderSettings::SpLiSP:
				m_shadowProjection0 = new LiSPShadowProjection();
				break;

			case WorldRenderSettings::SpTrapezoid:
				m_shadowProjection0 = new TrapezoidShadowProjection();
				break;

			default:
			case WorldRenderSettings::SpUniform:
				m_shadowProjection0 = new UniformShadowProjection(resolution);
				break;
			}

			m_shadowProjection = new UniformShadowProjection(resolution);
		}

		// Ensure targets are destroyed if something went wrong in setup.
		if (m_shadowsQuality == QuDisabled)
		{
			safeDestroy(m_shadowTargetSet);
			safeDestroy(m_shadowMaskProjectTargetSet);
		}
	}

	// Create "color read-back" copy processing.
	{
		resource::Proxy< PostProcessSettings > colorTargetCopy;

		if (!resourceManager->bind(c_colorTargetCopy, colorTargetCopy))
			log::warning << L"Unable to create color read-back processing; color read-back disabled" << Endl;

		if (colorTargetCopy)
		{
			m_colorTargetCopy = new world::PostProcess();
			if (!m_colorTargetCopy->create(
				colorTargetCopy,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				width,
				height
			))
			{
				log::warning << L"Unable to create color read-back processing; color read-back disabled" << Endl;
				m_colorTargetCopy = 0;
			}
		}
	}

	// Create ambient occlusion processing.
	{
		resource::Id< PostProcessSettings > ambientOcclusionId;
		resource::Proxy< PostProcessSettings > ambientOcclusion;

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
			m_ambientOcclusion = new PostProcess();
			if (!m_ambientOcclusion->create(
				ambientOcclusion,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				width,
				height
			))
			{
				log::warning << L"Unable to create ambient occlusion process; AO disabled" << Endl;
				m_ambientOcclusion = 0;
			}
		}
	}

	// Create antialias processing.
	{
		resource::Id< PostProcessSettings > antiAliasId;
		resource::Proxy< PostProcessSettings > antiAlias;

		switch (m_antiAliasQuality)
		{
		default:
		case QuDisabled:
			antiAliasId = c_antiAliasNone;
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
			m_antiAlias = new PostProcess();
			if (!m_antiAlias->create(
				antiAlias,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				width,
				height
			))
			{
				log::warning << L"Unable to create antialias process; AA disabled" << Endl;
				m_antiAlias = 0;
			}
		}
	}

	// Create "visual" post processing filter.
	if (desc.postProcessSettings)
	{
		m_visualPostProcess = new world::PostProcess();
		if (!m_visualPostProcess->create(
			desc.postProcessSettings,
			postProcessTargetPool,
			resourceManager,
			renderSystem,
			width,
			height
		))
		{
			log::warning << L"Unable to create visual post processing; post processing disabled" << Endl;
			m_visualPostProcess = 0;
		}
	}

	// Create gamma correction processing.
	if (m_settings.linearLighting)
	{
		resource::Proxy< PostProcessSettings > gammaCorrection;
		if (!resourceManager->bind(c_gammaCorrection, gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled" << Endl;

		if (gammaCorrection)
		{
			m_gammaCorrectionPostProcess = new PostProcess();
			if (!m_gammaCorrectionPostProcess->create(
				gammaCorrection,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				width,
				height
			))
			{
				log::warning << L"Unable to create gamma correction process; gamma correction disabled" << Endl;
				m_gammaCorrectionPostProcess = 0;
			}
		}
	}

	// Create global reflection map.
	if (m_settings.reflectionMap)
	{
		if (!resourceManager->bind(m_settings.reflectionMap, m_reflectionMap))
			log::warning << L"Unable to create reflection map" << Endl;
	}

	// Create "visual" and "intermediate" target.
	{
		render::RenderTargetSetCreateDesc desc;
		
		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = desc.multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR11G11B10F;

		m_visualTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_visualTargetSet)
			return false;

		m_intermediateTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_intermediateTargetSet)
			return false;
	}

	// Create software rastering cullers.
	if (m_settings.occlusionCullingEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->culling = new WorldCullingSwRaster();
	}

	// Allocate "gbuffer" context.
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->gbuffer = new WorldContext(desc.entityRenderers, i->culling);
	}

	// Allocate "shadow" contexts.
	if (m_shadowsQuality > QuDisabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		{
			for (int32_t j = 0; j < m_shadowSettings.cascadingSlices; ++j)
			{
				for (int32_t k = 0; k < MaxLightShadowCount; ++k)
					i->slice[j].shadow[k] = new WorldContext(desc.entityRenderers, 0);
			}
		}
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(desc.entityRenderers, i->culling);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	// Create light primitive renderer.
	m_lightRenderer = new LightRendererDeferred();
	if (!m_lightRenderer->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create light primitive renderer" << Endl;
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

	m_count = 0;
	return true;
}

void WorldRendererDeferred::destroy()
{
	m_frames.clear();
	m_buildEntities.clear();

	safeDestroy(m_lightRenderer);
	safeDestroy(m_gammaCorrectionPostProcess);
	safeDestroy(m_visualPostProcess);
	safeDestroy(m_antiAlias);
	safeDestroy(m_ambientOcclusion);
	safeDestroy(m_colorTargetCopy);
	safeDestroy(m_shadowMaskFilter);
	safeDestroy(m_shadowMaskProject);

	m_reflectionMap.clear();
	m_globalContext = 0;

	safeDestroy(m_shadowMaskFilterTargetSet);
	safeDestroy(m_shadowMaskProjectTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_colorTargetSet);
	safeDestroy(m_gbufferTargetSet);
	safeDestroy(m_intermediateTargetSet);
	safeDestroy(m_visualTargetSet);

	m_shadowProjection = 0;
	m_shadowProjection0 = 0;
	m_renderView = 0;
}

void WorldRendererDeferred::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
{
	float viewNearZ = m_settings.viewNearZ;
	float viewFarZ = m_settings.viewFarZ;

	Frustum viewFrustum;
	viewFrustum.buildPerspective(worldView.fov, worldView.aspect, viewNearZ, viewFarZ);

	outRenderView.setViewSize(Vector2(float(worldView.width), float(worldView.height)));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(perspectiveLh(worldView.fov, worldView.aspect, viewNearZ, viewFarZ));
}

void WorldRendererDeferred::createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const
{
	float viewFarZ = m_settings.viewFarZ;

	Frustum viewFrustum;
	viewFrustum.buildOrtho(worldView.width, worldView.height, -viewFarZ, viewFarZ);

	outRenderView.setViewSize(Vector2(worldView.width, worldView.height));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(orthoLh(worldView.width, worldView.height, -viewFarZ, viewFarZ));
}

bool WorldRendererDeferred::beginBuild()
{
	return true;
}

void WorldRendererDeferred::build(Entity* entity)
{
	m_buildEntities.push_back(entity);
}

void WorldRendererDeferred::endBuild(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];

	// Flush previous frame.
	f.gbuffer->clear();

	uint32_t shadowLightCount = min< uint32_t >(f.lightCount, MaxLightShadowCount);
	for (uint32_t i = 0; i < shadowLightCount; ++i)
	{
		for (int32_t j = 0; j < m_shadowSettings.cascadingSlices; ++j)
		{
			if (f.slice[j].shadow[i])
				f.slice[j].shadow[i]->clear();
		}
	}

	f.visual->clear();

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	worldRenderView.setEyePosition(viewInverse.translation().xyz1());
	worldRenderView.setEyeDirection(viewInverse.axisZ().xyz0());

	// Store some global values.
	f.time = worldRenderView.getTime();

	// Prepare occluders.
	if (f.culling)
	{
		f.culling->beginPrecull(worldRenderView);
		for (RefArray< Entity >::const_iterator i = m_buildEntities.begin(); i != m_buildEntities.end(); ++i)
			f.gbuffer->precull(worldRenderView, *i);
		f.culling->endPrecull();
	}

	// Build gbuffer context.
	{
		WorldRenderView gbufferRenderView = worldRenderView;
		gbufferRenderView.resetLights();

		WorldRenderPassDeferred gbufferPass(
			ms_techniqueDeferredGBufferWrite,
			gbufferRenderView
		);
		for (RefArray< Entity >::const_iterator i = m_buildEntities.begin(); i != m_buildEntities.end(); ++i)
			f.gbuffer->build(gbufferRenderView, gbufferPass, *i);
		f.gbuffer->flush(gbufferRenderView, gbufferPass);

		f.haveGBuffer = true;
	}

	// Build shadow contexts.
	if (m_shadowsQuality > QuDisabled)
		buildLightWithShadows(worldRenderView, frame);
	else
		buildLightWithNoShadows(worldRenderView, frame);

	// Build visual context.
	worldRenderView.resetLights();
	buildVisual(worldRenderView, frame);

	m_buildEntities.resize(0);
	m_count++;
}

bool WorldRendererDeferred::beginRender(int frame, render::EyeType eye, const Color4f& clearColor)
{
	if (!m_renderView->begin(m_visualTargetSet, 0))
		return false;

	m_renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);
	return true;
}

void WorldRendererDeferred::render(uint32_t flags, int frame, render::EyeType eye)
{
	Frame& f = m_frames[frame];
	Matrix44 projection = f.projection;

	// Render gbuffer.
	if ((flags & (WrfDepthMap | WrfNormalMap | WrfVisualOpaque)) != 0)
	{
		render::ProgramParameters gbufferProgramParams;
		gbufferProgramParams.beginParameters(m_globalContext);
		gbufferProgramParams.setFloatParameter(ms_handleTime, f.time);
		gbufferProgramParams.setMatrixParameter(ms_handleView, f.view);
		gbufferProgramParams.setMatrixParameter(ms_handleViewInverse, f.view.inverse());
		gbufferProgramParams.setMatrixParameter(ms_handleProjection, projection);
		gbufferProgramParams.endParameters(m_globalContext);

		T_RENDER_PUSH_MARKER(m_renderView, "World: GBuffer");
		if (m_renderView->begin(m_gbufferTargetSet))
		{
			const float clearZ = std::numeric_limits< float >::max();

			const Color4f depthColor(clearZ, clearZ, clearZ, clearZ);
			const Color4f normalColor(0.5f, 0.5f, 0.0f, 0.5f);
			const Color4f surfaceColor(0.0f, 0.0f, 0.0f, 0.0f);
			const Color4f clearColors[] = { depthColor, normalColor, surfaceColor };

			m_renderView->clear(render::CfColor | render::CfDepth, clearColors, 1.0f, 0);

			if (f.haveGBuffer)
				f.gbuffer->getRenderContext()->render(m_renderView, render::RpOpaque, &gbufferProgramParams);

			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render shadow and light maps.
	if ((flags & (WrfShadowMap | WrfLightMap)) != 0 && f.lightCount > 0)
	{
		bool firstLight = true;

		// First render all shadowing lights.
		if ((flags & WrfShadowMap) != 0)
		{
			uint32_t shadowLightCount = min< uint32_t >(f.lightCount, MaxLightShadowCount);
			for (uint32_t i = 0; i < shadowLightCount; ++i)
			{
				if (!f.haveShadows[i])
					continue;

				// Combine all shadow slices into a screen shadow mask.
				for (int32_t j = 0; j < m_shadowSettings.cascadingSlices; ++j)
				{
					render::ProgramParameters shadowProgramParams;
					shadowProgramParams.beginParameters(m_globalContext);
					shadowProgramParams.setFloatParameter(ms_handleTime, f.time);
					shadowProgramParams.setMatrixParameter(ms_handleView, f.slice[j].shadowLightView[i]);
					shadowProgramParams.setMatrixParameter(ms_handleViewInverse, f.slice[j].shadowLightView[i].inverse());
					shadowProgramParams.setMatrixParameter(ms_handleProjection, f.slice[j].shadowLightProjection[i]);
					shadowProgramParams.setMatrixParameter(ms_handleSquareProjection, f.slice[j].shadowLightSquareProjection[i]);
					shadowProgramParams.endParameters(m_globalContext);

					T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
					if (m_renderView->begin(m_shadowTargetSet, 0))
					{
						const Color4f shadowClear(1.0f, 1.0f, 1.0f, 1.0f);
						m_renderView->clear(render::CfColor | render::CfDepth, &shadowClear, 1.0f, 0);
						f.slice[j].shadow[i]->getRenderContext()->render(m_renderView, render::RpOpaque, &shadowProgramParams);
						m_renderView->end();
					}
					T_RENDER_POP_MARKER(m_renderView);

					T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask project");
					if (m_renderView->begin(m_shadowMaskProjectTargetSet, 0))
					{
						if (j == 0)
						{
							const Color4f maskClear(1.0f, 1.0f, 1.0f, 1.0f);
							m_renderView->clear(render::CfColor, &maskClear, 0.0f, 0);
						}

						Scalar zn(max(m_slicePositions[j], m_settings.viewNearZ));
						Scalar zf(min(m_slicePositions[j + 1], m_shadowSettings.farZ));

						PostProcessStep::Instance::RenderParams params;
						params.viewFrustum = f.viewFrustum;
						params.viewToLight = f.slice[j].viewToLightSpace[i];
						params.projection = projection;
						params.sliceCount = m_shadowSettings.cascadingSlices;
						params.sliceIndex = j;
						params.sliceNearZ = zn;
						params.sliceFarZ = zf;
						params.shadowFarZ = m_shadowSettings.farZ;
						params.shadowMapBias = m_shadowSettings.bias + i * m_shadowSettings.biasCoeff;
						params.deltaTime = 0.0f;

						m_shadowMaskProject->render(
							m_renderView,
							m_shadowTargetSet,
							m_gbufferTargetSet,
							0,
							params
						);

						m_renderView->end();
					}
					T_RENDER_POP_MARKER(m_renderView);
				}

				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask filter");
				if (m_renderView->begin(m_shadowMaskFilterTargetSet, 0))
				{
					const Color4f maskClear(1.0f, 1.0f, 1.0f, 1.0f);
					m_renderView->clear(render::CfColor, &maskClear, 0.0f, 0);

					PostProcessStep::Instance::RenderParams params;
					params.viewFrustum = f.viewFrustum;
					params.projection = projection;
					params.sliceNearZ = 0.0f;
					params.sliceFarZ = m_shadowSettings.farZ;
					params.shadowMapBias = m_shadowSettings.bias;
					params.deltaTime = 0.0f;

					m_shadowMaskFilter->render(
						m_renderView,
						m_shadowMaskProjectTargetSet,
						m_gbufferTargetSet,
						0,
						params
					);
					m_renderView->end();
				}
				T_RENDER_POP_MARKER(m_renderView);

				if ((flags & WrfLightMap) != 0)
				{
					T_RENDER_PUSH_MARKER(m_renderView, "World: Light primitive (shadow)");
					if (m_renderView->begin(m_visualTargetSet, 0))
					{
						if (firstLight)
						{
							const Color4f lightClear(0.0f, 0.0f, 0.0f, 0.0f);
							m_renderView->clear(render::CfColor, &lightClear, 0.0f, 0);
							firstLight = false;
						}

						m_lightRenderer->renderLight(
							m_renderView,
							projection,
							f.view,
							f.lights[i],
							m_gbufferTargetSet->getColorTexture(0),
							m_gbufferTargetSet->getColorTexture(1),
							m_gbufferTargetSet->getColorTexture(2),
							m_shadowMaskFilterTargetSet->getWidth(),
							m_shadowMaskFilterTargetSet->getColorTexture(0)
						);
						m_renderView->end();
					}
					T_RENDER_POP_MARKER(m_renderView);
				}
			}
		}

		// Then render all non-shadowing lights; no need to rebind render target for each light.
		if ((flags & WrfLightMap) != 0)
		{
			if (m_renderView->begin(m_visualTargetSet, 0))
			{
				if (firstLight)
				{
					const Color4f lightClear(0.0f, 0.0f, 0.0f, 0.0f);
					m_renderView->clear(render::CfColor, &lightClear, 0.0f, 0);
					firstLight = false;
				}

				for (uint32_t i = 0; i < f.lightCount; ++i)
				{
					if (f.haveShadows[i])
						continue;

					T_RENDER_PUSH_MARKER(m_renderView, "World: Light primitive (no shadow)");
					m_lightRenderer->renderLight(
						m_renderView,
						projection,
						f.view,
						f.lights[i],
						m_gbufferTargetSet->getColorTexture(0),
						m_gbufferTargetSet->getColorTexture(1),
						m_gbufferTargetSet->getColorTexture(2),
						0,
						0
					);
					T_RENDER_POP_MARKER(m_renderView);
				}

				m_lightRenderer->renderFinal(
					m_renderView,
					projection,
					m_fogDistanceAndDensity,
					m_fogColor,
					m_gbufferTargetSet->getColorTexture(0),
					m_gbufferTargetSet->getColorTexture(1),
					m_gbufferTargetSet->getColorTexture(2)
				);

				m_renderView->end();
			}
		}
	}
	else
	{
		// No active lights; ensure light map is cleared.
		if ((flags & WrfLightMap) != 0)
		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: Light map");
			if (m_renderView->begin(m_visualTargetSet, 0))
			{
				const Color4f lightClear(0.0f, 0.0f, 0.0f, 0.0f);
				m_renderView->clear(render::CfColor, &lightClear, 0.0f, 0);
				m_renderView->end();
			}
			T_RENDER_POP_MARKER(m_renderView);
		}
	}

	// Render opaque visuals.
	if ((flags & WrfVisualOpaque) != 0)
	{
		render::ProgramParameters visualProgramParams;
		visualProgramParams.beginParameters(m_globalContext);
		visualProgramParams.setFloatParameter(ms_handleTime, f.time);
		visualProgramParams.setVectorParameter(ms_handleFogDistanceAndDensity, m_fogDistanceAndDensity);
		visualProgramParams.setVectorParameter(ms_handleFogColor, m_fogColor);
		visualProgramParams.setMatrixParameter(ms_handleView, f.view);
		visualProgramParams.setMatrixParameter(ms_handleViewInverse, f.view.inverse());
		visualProgramParams.setMatrixParameter(ms_handleProjection, projection);
		visualProgramParams.setTextureParameter(ms_handleColorMap, m_colorTargetSet->getColorTexture(0));
		visualProgramParams.setTextureParameter(ms_handleDepthMap, m_gbufferTargetSet->getColorTexture(0));
		visualProgramParams.setTextureParameter(ms_handleNormalMap, m_gbufferTargetSet->getColorTexture(1));
		visualProgramParams.setTextureParameter(ms_handleReflectionMap, m_reflectionMap);
		visualProgramParams.endParameters(m_globalContext);

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual opaque");
		f.visual->getRenderContext()->render(m_renderView, render::RpSetup | render::RpOpaque, &visualProgramParams);
		T_RENDER_POP_MARKER(m_renderView);

		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: Color read-back copy");
			m_renderView->end();

			m_renderView->begin(m_colorTargetSet, 0);

			PostProcessStep::Instance::RenderParams params;
			params.viewFrustum = f.viewFrustum;
			params.projection = projection;
			params.deltaTime = 0.0f;

			m_colorTargetCopy->render(
				m_renderView,
				m_visualTargetSet,
				0,
				0,
				params
			);

			m_renderView->end();

			m_renderView->begin(m_visualTargetSet, 0);
			T_RENDER_POP_MARKER(m_renderView);
		}

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual post opaque");
		f.visual->getRenderContext()->render(m_renderView, render::RpPostOpaque, &visualProgramParams);
		T_RENDER_POP_MARKER(m_renderView);

		if (m_ambientOcclusion)
		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: AO");

			PostProcessStep::Instance::RenderParams params;
			params.viewFrustum = f.viewFrustum;
			params.projection = projection;
			params.deltaTime = 0.0f;

			m_ambientOcclusion->render(
				m_renderView,
				m_shadowTargetSet,
				m_gbufferTargetSet,
				0,
				params
			);

			T_RENDER_POP_MARKER(m_renderView);
		}
	}

	// Render alpha blend visuals.
	if ((flags & (WrfVisualAlphaBlend)) != 0)
	{
		render::ProgramParameters visualProgramParams;
		visualProgramParams.beginParameters(m_globalContext);
		visualProgramParams.setFloatParameter(ms_handleTime, f.time);
		visualProgramParams.setVectorParameter(ms_handleFogDistanceAndDensity, m_fogDistanceAndDensity);
		visualProgramParams.setVectorParameter(ms_handleFogColor, m_fogColor);
		visualProgramParams.setMatrixParameter(ms_handleView, f.view);
		visualProgramParams.setMatrixParameter(ms_handleViewInverse, f.view.inverse());
		visualProgramParams.setMatrixParameter(ms_handleProjection, projection);
		visualProgramParams.setTextureParameter(ms_handleColorMap, m_colorTargetSet->getColorTexture(0));
		visualProgramParams.setTextureParameter(ms_handleDepthMap, m_gbufferTargetSet->getColorTexture(0));
		visualProgramParams.setTextureParameter(ms_handleNormalMap, m_gbufferTargetSet->getColorTexture(1));
		visualProgramParams.setTextureParameter(ms_handleReflectionMap, m_reflectionMap);
		visualProgramParams.endParameters(m_globalContext);

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual alpha blend");
		f.visual->getRenderContext()->render(m_renderView, render::RpAlphaBlend, &visualProgramParams);
		T_RENDER_POP_MARKER(m_renderView);

		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: Color read-back copy");
			m_renderView->end();

			m_renderView->begin(m_colorTargetSet, 0);

			PostProcessStep::Instance::RenderParams params;
			params.viewFrustum = f.viewFrustum;
			params.projection = projection;
			params.deltaTime = 0.0f;

			m_colorTargetCopy->render(
				m_renderView,
				m_visualTargetSet,
				0,
				0,
				params
			);

			m_renderView->end();

			m_renderView->begin(m_visualTargetSet, 0);
			T_RENDER_POP_MARKER(m_renderView);
		}

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual post alpha blend");
		f.visual->getRenderContext()->render(m_renderView, render::RpPostAlphaBlend | render::RpOverlay, &visualProgramParams);
		T_RENDER_POP_MARKER(m_renderView);
	}

	m_globalContext->flush();
}

void WorldRendererDeferred::endRender(int frame, render::EyeType eye, float deltaTime)
{
	Frame& f = m_frames[frame];

	world::PostProcessStep::Instance::RenderParams params;
	params.viewFrustum = f.viewFrustum;
	params.viewToLight = Matrix44::identity(); //f.viewToLightSpace;
	params.view = f.view;
	params.projection = f.projection;
	params.godRayDirection = f.godRayDirection;
	params.deltaTime = deltaTime;

	m_renderView->end();

	render::RenderTargetSet* sourceTargetSet = m_visualTargetSet;
	render::RenderTargetSet* outputTargetSet = m_intermediateTargetSet;
	T_ASSERT (sourceTargetSet);

	// Apply custom post processing filter.
	if (m_visualPostProcess)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Custom PP");

		if (m_gammaCorrectionPostProcess || m_antiAlias)
			m_renderView->begin(outputTargetSet);

		m_visualPostProcess->render(
			m_renderView,
			sourceTargetSet,
			m_gbufferTargetSet,
			m_shadowTargetSet,
			params
		);

		if (m_gammaCorrectionPostProcess || m_antiAlias)
		{
			m_renderView->end();
			std::swap(sourceTargetSet, outputTargetSet);
		}

		T_RENDER_POP_MARKER(m_renderView);
	}

	// Apply gamma correction filter.
	if (m_gammaCorrectionPostProcess)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Gamma Correction");

		if (m_antiAlias)
			m_renderView->begin(outputTargetSet);

		m_gammaCorrectionPostProcess->render(
			m_renderView,
			sourceTargetSet,
			m_gbufferTargetSet,
			m_shadowTargetSet,
			params
		);

		if (m_antiAlias)
		{
			m_renderView->end();
			std::swap(sourceTargetSet, outputTargetSet);
		}

		T_RENDER_POP_MARKER(m_renderView);
	}

	// Apply software antialias filter.
	if (m_antiAlias)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: AntiAlias");

		m_antiAlias->render(
			m_renderView,
			sourceTargetSet,
			m_gbufferTargetSet,
			m_shadowTargetSet,
			params
		);

		T_RENDER_POP_MARKER(m_renderView);
	}
}

PostProcess* WorldRendererDeferred::getVisualPostProcess()
{
	return m_visualPostProcess;
}

render::RenderTargetSet* WorldRendererDeferred::getVisualTargetSet()
{
	return m_visualTargetSet;
}

render::RenderTargetSet* WorldRendererDeferred::getDepthTargetSet()
{
	return m_gbufferTargetSet;
}

render::RenderTargetSet* WorldRendererDeferred::getShadowMaskTargetSet()
{
	return m_shadowMaskFilterTargetSet;
}

void WorldRendererDeferred::getTargets(RefArray< render::ITexture >& outTargets) const
{
	outTargets.resize(5);
	outTargets[0] = m_gbufferTargetSet ? m_gbufferTargetSet->getColorTexture(0) : 0;
	outTargets[1] = m_gbufferTargetSet ? m_gbufferTargetSet->getColorTexture(1) : 0;
	outTargets[2] = m_gbufferTargetSet ? m_gbufferTargetSet->getColorTexture(2) : 0;
	outTargets[4] = m_shadowMaskFilterTargetSet ? m_shadowMaskFilterTargetSet->getColorTexture(0) : 0;
}

void WorldRendererDeferred::buildLightWithShadows(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();

	f.lightCount = worldRenderView.getLightCount();

	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);

		f.lights[i] = light;

		if (
			(light.type == LtDirectional || light.type == LtSpot) &&
			light.castShadow &&
			i < MaxLightShadowCount
		)
		{
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
				Matrix44 shadowLightSquareProjection = Matrix44::identity();
				Frustum shadowFrustum;

				(slice == 0 ? m_shadowProjection0 : m_shadowProjection)->calculate(
					viewInverse,
					light.position,
					light.direction,
					sliceViewFrustum,
					shadowBox,
					m_shadowSettings.farZ,
					m_shadowSettings.quantizeProjection,
					shadowLightView,
					shadowLightProjection,
					shadowLightSquareProjection,
					shadowFrustum
				);

				// Render shadow map.
				WorldRenderView shadowRenderView;
				shadowRenderView.resetLights();
				shadowRenderView.setProjection(shadowLightProjection);
				shadowRenderView.setView(shadowLightView);
				shadowRenderView.setViewFrustum(shadowFrustum);
				shadowRenderView.setCullFrustum(shadowFrustum);
				shadowRenderView.setEyePosition(worldRenderView.getEyePosition());
				shadowRenderView.setTimes(
					worldRenderView.getTime(),
					worldRenderView.getDeltaTime(),
					worldRenderView.getInterval()
				);

				WorldRenderPassDeferred shadowPass(
					ms_techniqueShadow,
					shadowRenderView
				);
				for (RefArray< Entity >::const_iterator j = m_buildEntities.begin(); j != m_buildEntities.end(); ++j)
					f.slice[slice].shadow[i]->build(shadowRenderView, shadowPass, *j);
				f.slice[slice].shadow[i]->flush(shadowRenderView, shadowPass);
				
				f.slice[slice].shadowLightView[i] = shadowLightView;
				f.slice[slice].shadowLightProjection[i] = shadowLightProjection;
				f.slice[slice].shadowLightSquareProjection[i] = shadowLightSquareProjection;
				f.slice[slice].viewToLightSpace[i] = shadowLightSquareProjection * shadowLightProjection * shadowLightView * viewInverse;
			}

			f.haveShadows[i] = true;
		}
		else
			f.haveShadows[i] = false;
	}
}

void WorldRendererDeferred::buildLightWithNoShadows(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];
	f.lightCount = worldRenderView.getLightCount();
	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);
		f.lights[i] = light;
	}
}

void WorldRendererDeferred::buildVisual(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];

	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();

	WorldRenderPassDeferred defaultPreLitPass(
		ms_techniqueDeferredColor,
		worldRenderView,
		m_settings.fogEnabled,
		m_gbufferTargetSet->getColorTexture(0) != 0
	);
	for (RefArray< Entity >::const_iterator i = m_buildEntities.begin(); i != m_buildEntities.end(); ++i)
		f.visual->build(worldRenderView, defaultPreLitPass, *i);
	f.visual->flush(worldRenderView, defaultPreLitPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.godRayDirection = worldRenderView.getGodRayDirection();
}

	}
}
