#include "Core/Log/Log.h"
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
#include "World/WorldContext.h"
#include "World/WorldCullingSwRaster.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderView.h"
#include "World/Entity/Entity.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PreLit/LightRenderer.h"
#include "World/PreLit/WorldRendererPreLit.h"
#include "World/PreLit/WorldRenderPassPreLit.h"
#include "World/SMProj/BoxShadowProjection.h"
#include "World/SMProj/LiSPShadowProjection.h"
#include "World/SMProj/TrapezoidShadowProjection.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const resource::Id< PostProcessSettings > c_shadowMaskProject(Guid(L"{F751F3EB-33D2-9247-9E3F-54B1A0E3522C}"));
const resource::Id< PostProcessSettings > c_shadowMaskFilterNone(Guid(L"{19222311-363F-CB45-86E5-34D376CDA8AD}"));
const resource::Id< PostProcessSettings > c_shadowMaskFilterLow(Guid(L"{7D4D38B9-1E43-8046-B1A4-705CFEF9B8EB}"));
const resource::Id< PostProcessSettings > c_shadowMaskFilterMedium(Guid(L"{57FD53AF-547A-9F46-8C94-B4D24EFB63BC}"));
const resource::Id< PostProcessSettings > c_shadowMaskFilterHigh(Guid(L"{FABC4017-4D65-604D-B9AB-9FC03FE3CE43}"));
const resource::Id< PostProcessSettings > c_shadowMaskFilterHighest(Guid(L"{5AFC153E-6FCE-3142-9E1B-DD3722DA447F}"));

const resource::Id< PostProcessSettings > c_ambientOcclusionLow(Guid(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}"));		// SSAO, half size
const resource::Id< PostProcessSettings > c_ambientOcclusionMedium(Guid(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}"));	// SSAO, full size
const resource::Id< PostProcessSettings > c_ambientOcclusionHigh(Guid(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}"));		// HBAO, half size
const resource::Id< PostProcessSettings > c_ambientOcclusionHighest(Guid(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}"));	// HBAO, full size

const resource::Id< PostProcessSettings > c_antiAliasNone(Guid(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}"));
const resource::Id< PostProcessSettings > c_antiAliasLow(Guid(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}"));
const resource::Id< PostProcessSettings > c_antiAliasMedium(Guid(L"{3E1D810B-339A-F742-9345-4ECA00220D57}"));
const resource::Id< PostProcessSettings > c_antiAliasHigh(Guid(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}"));
const resource::Id< PostProcessSettings > c_antiAliasHighest(Guid(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}"));

const resource::Id< PostProcessSettings > c_gammaCorrection(Guid(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}"));

const static float c_interocularDistance = 6.5f;
const static float c_distortionValue = 0.8f;
const static float c_screenPlaneDistance = 13.0f;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererPreLit", 0, WorldRendererPreLit, IWorldRenderer)

render::handle_t WorldRendererPreLit::ms_techniquePreLitColor = 0;
render::handle_t WorldRendererPreLit::ms_techniqueGBuffer = 0;
render::handle_t WorldRendererPreLit::ms_techniqueShadow = 0;
render::handle_t WorldRendererPreLit::ms_handleProjection = 0;

WorldRendererPreLit::WorldRendererPreLit()
:	m_count(0)
{
	// Techniques
	ms_techniquePreLitColor = render::getParameterHandle(L"World_PreLitColor");
	ms_techniqueGBuffer = render::getParameterHandle(L"World_GBufferWrite");
	ms_techniqueShadow = render::getParameterHandle(L"World_ShadowWrite");

	// Global parameters.
	ms_handleProjection = render::getParameterHandle(L"Projection");
}

bool WorldRendererPreLit::create(
	const WorldRenderSettings* worldRenderSettings,
	const PostProcessSettings* postProcessSettings,
	WorldEntityRenderers* entityRenderers,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::IRenderView* renderView,
	uint32_t multiSample,
	uint32_t frameCount
)
{
	T_ASSERT_M (worldRenderSettings, L"World render settings required");
	T_ASSERT_M (renderView, L"Render view required");

	m_settings = *worldRenderSettings;
	m_renderView = renderView;
	m_frames.resize(frameCount);

	float fogColor[4];
	m_settings.fogColor.getRGBA32F(fogColor);
	m_fogColor = Vector4::loadUnaligned(fogColor);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create "gbuffer" targets.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 2;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR16F;		// Depth
		desc.targets[1].format = render::TfR8G8B8A8;	// Normals

		m_gbufferTargetSet = renderSystem->createRenderTargetSet(desc);

		if (!m_gbufferTargetSet && multiSample > 0)
		{
			desc.multiSample = 0;
			desc.createDepthStencil = true;
			desc.usingPrimaryDepthStencil = false;

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

	// Create "shadow map" targets.
	if (m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		uint32_t shadowMapResolution = m_settings.shadowMapResolution;
		switch (m_settings.shadowsQuality)
		{
		case WorldRenderSettings::SqLow:
			shadowMapResolution /= 4;
			break;

		case WorldRenderSettings::SqMedium:
			shadowMapResolution /= 2;
			break;

		default:
			break;
		}

		// Create shadow map target.
		desc.count = 1;
		desc.width =
		desc.height = shadowMapResolution;
		desc.multiSample = 0;
		desc.createDepthStencil = true;
		desc.usingPrimaryDepthStencil = false;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR16F;
		m_shadowTargetSet = renderSystem->createRenderTargetSet(desc);

		// Determine shadow mask size; high quality is same as entire screen.
		if (
			m_settings.shadowsQuality == WorldRenderSettings::SqNoFilter ||
			m_settings.shadowsQuality == WorldRenderSettings::SqHigh ||
			m_settings.shadowsQuality == WorldRenderSettings::SqHighest
		)
		{
			desc.width = width;
			desc.height = height;
		}
		else
		{
			desc.width = width / 2;
			desc.height = height / 2;
		}

		// Create shadow mask target.
		desc.count = 1;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8;
		desc.preferTiled = true;
		m_shadowMaskProjectTargetSet = renderSystem->createRenderTargetSet(desc);

		// Create filtered shadow mask targets.
		m_shadowMaskFilterTargetSet.resize(MaxLightCount);
		for (int i = 0; i < MaxLightCount; ++i)
		{
			desc.count = 1;
			desc.multiSample = 0;
			desc.createDepthStencil = false;
			desc.usingPrimaryDepthStencil = false;
			desc.targets[0].format = render::TfR8;
			desc.preferTiled = true;
			m_shadowMaskFilterTargetSet[i] = renderSystem->createRenderTargetSet(desc);
			if (!m_shadowMaskFilterTargetSet[i])
			{
				safeDestroy(m_shadowMaskProjectTargetSet);
				m_shadowMaskFilterTargetSet.clear();
				break;
			}
		}

		if (
			m_shadowTargetSet &&
			m_shadowMaskProjectTargetSet
		)
		{
			resource::Id< PostProcessSettings > shadowMaskProjectId;
			resource::Id< PostProcessSettings > shadowMaskFilterId;

			shadowMaskProjectId = c_shadowMaskProject;

			switch (m_settings.shadowsQuality)
			{
			case WorldRenderSettings::SqNoFilter:
				shadowMaskFilterId = c_shadowMaskFilterNone;
				break;
			case WorldRenderSettings::SqLow:
				shadowMaskFilterId = c_shadowMaskFilterLow;
				break;
			case WorldRenderSettings::SqMedium:
				shadowMaskFilterId = c_shadowMaskFilterMedium;
				break;
			case WorldRenderSettings::SqHigh:
				shadowMaskFilterId = c_shadowMaskFilterHigh;
				break;
			case WorldRenderSettings::SqHighest:
				shadowMaskFilterId = c_shadowMaskFilterHighest;
				break;
			}

			resource::Proxy< PostProcessSettings > shadowMaskProject;
			resource::Proxy< PostProcessSettings > shadowMaskFilter;

			if (
				!resourceManager->bind(shadowMaskProjectId, shadowMaskProject) ||
				!resourceManager->bind(shadowMaskFilterId, shadowMaskFilter)
			)
			{
				log::warning << L"Unable to create shadow project process; shadows disabled (1)" << Endl;
				m_settings.shadowsEnabled = false;
			}

			if (m_settings.shadowsEnabled)
			{
				m_shadowMaskProject = new PostProcess();
				if (!m_shadowMaskProject->create(
					shadowMaskProject,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height
				))
				{
					log::warning << L"Unable to create shadow project process; shadows disabled" << Endl;
					m_settings.shadowsEnabled = false;
				}

				m_shadowMaskFilter = new PostProcess();
				if (!m_shadowMaskFilter->create(
					shadowMaskFilter,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height
				))
				{
					log::warning << L"Unable to create shadow filter process; shadows disabled" << Endl;
					m_settings.shadowsEnabled = false;
				}
			}
		}
		else
		{
			log::warning << L"Unable to create shadow render targets; shadows disabled" << Endl;
			m_settings.shadowsEnabled = false;
		}

		if (m_settings.shadowsEnabled)
		{
			switch (m_settings.shadowsProjection)
			{
			case WorldRenderSettings::SpBox:
				m_shadowProjection = new BoxShadowProjection(m_settings);
				break;

			case WorldRenderSettings::SpLiSP:
				m_shadowProjection = new LiSPShadowProjection();
				break;

			case WorldRenderSettings::SpTrapezoid:
				m_shadowProjection = new TrapezoidShadowProjection(m_settings);
				break;

			default:
			case WorldRenderSettings::SpUniform:
				m_shadowProjection = new UniformShadowProjection(m_settings, shadowMapResolution);
				break;
			}
		}

		// Ensure targets are destroyed if something went wrong in setup.
		if (!m_settings.shadowsEnabled)
		{
			safeDestroy(m_shadowTargetSet);
			safeDestroy(m_shadowMaskProjectTargetSet);
			m_shadowMaskFilterTargetSet.clear();
		}
	}

	// Create ambient occlusion processing.
	{
		resource::Id< PostProcessSettings > ambientOcclusionId;
		resource::Proxy< PostProcessSettings > ambientOcclusion;

		switch (m_settings.ambientOcclusionQuality)
		{
		case WorldRenderSettings::AoqLow:
			ambientOcclusionId = c_ambientOcclusionLow;
			break;

		case WorldRenderSettings::AoqMedium:
			ambientOcclusionId = c_ambientOcclusionMedium;
			break;

		case WorldRenderSettings::AoqHigh:
			ambientOcclusionId = c_ambientOcclusionHigh;
			break;

		case WorldRenderSettings::AoqHighest:
			ambientOcclusionId = c_ambientOcclusionHighest;
			break;

		default:
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

		switch (m_settings.antiAliasQuality)
		{
		default:
		case WorldRenderSettings::AaqDisabled:
			antiAliasId = c_antiAliasNone;
			break;

		case WorldRenderSettings::AaqLow:
			antiAliasId = c_antiAliasLow;
			break;

		case WorldRenderSettings::AoqMedium:
			antiAliasId = c_antiAliasMedium;
			break;

		case WorldRenderSettings::AoqHigh:
			antiAliasId = c_antiAliasHigh;
			break;

		case WorldRenderSettings::AoqHighest:
			antiAliasId = c_antiAliasHighest;
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
	if (postProcessSettings)
	{
		m_visualPostProcess = new world::PostProcess();
		if (!m_visualPostProcess->create(
			postProcessSettings,
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

	// Create "visual" and "intermediate" target.
	{
		render::RenderTargetSetCreateDesc desc;
		
		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
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

	// Create light map target.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR16G16B16A16F;

		m_lightMapTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_lightMapTargetSet)
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
			i->gbuffer = new WorldContext(entityRenderers, i->culling);
	}

	// Allocate "shadow" contexts.
	if (m_settings.shadowsEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		{
			for (int32_t j = 0; j < m_settings.shadowCascadingSlices; ++j)
			{
				for (int32_t k = 0; k < MaxLightCount; ++k)
					i->slice[j].shadow[k] = new WorldContext(entityRenderers, 0);
			}
		}
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(entityRenderers, i->culling);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	// Create light primitive renderer.
	m_lightRenderer = new LightRenderer();
	if (!m_lightRenderer->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create light primitive renderer" << Endl;
		return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < m_settings.shadowCascadingSlices; ++i)
	{
		float ii = float(i) / m_settings.shadowCascadingSlices;
		float log = powf(ii, m_settings.shadowCascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, m_settings.shadowFarZ, log);
	}
	m_slicePositions[m_settings.shadowCascadingSlices] = m_settings.shadowFarZ;

	m_count = 0;
	return true;
}

void WorldRendererPreLit::destroy()
{
	safeDestroy(m_lightRenderer);

	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		for (int32_t j = 0; j < MaxSliceCount; ++j)
		{
			for (int32_t k = 0; k < MaxLightCount; ++k)
				i->slice[j].shadow[k] = 0;
		}

		i->visual = 0;
		i->gbuffer = 0;
	}

	safeDestroy(m_gammaCorrectionPostProcess);
	safeDestroy(m_visualPostProcess);
	safeDestroy(m_antiAlias);
	safeDestroy(m_ambientOcclusion);
	safeDestroy(m_shadowMaskFilter);
	safeDestroy(m_shadowMaskProject);
	m_shadowMaskFilterTargetSet.clear();
	safeDestroy(m_shadowMaskProjectTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_lightMapTargetSet);
	safeDestroy(m_gbufferTargetSet);
	safeDestroy(m_intermediateTargetSet);
	safeDestroy(m_visualTargetSet);

	m_renderView = 0;
}

void WorldRendererPreLit::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
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

void WorldRendererPreLit::createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const
{
	float viewFarZ = m_settings.viewFarZ;

	Frustum viewFrustum;
	viewFrustum.buildOrtho(worldView.width, worldView.height, -viewFarZ, viewFarZ);

	outRenderView.setViewSize(Vector2(worldView.width, worldView.height));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(orthoLh(worldView.width, worldView.height, -viewFarZ, viewFarZ));
}

void WorldRendererPreLit::build(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	// Flush previous frame.
	f.gbuffer->clear();

	for (uint32_t i = 0; i < f.lightCount; ++i)
	{
		for (int32_t j = 0; j < m_settings.shadowCascadingSlices; ++j)
		{
			if (f.slice[j].shadow[i])
				f.slice[j].shadow[i]->clear();
		}
	}

	f.visual->clear();

	// Prepare occluders.
	if (f.culling)
	{
		f.culling->beginPrecull(worldRenderView);
		f.gbuffer->precull(worldRenderView, entity);
		f.culling->endPrecull();
	}

	// Build gbuffer context.
	{
		WorldRenderView gbufferRenderView = worldRenderView;
		gbufferRenderView.resetLights();

		WorldRenderPassPreLit gbufferPass(
			ms_techniqueGBuffer,
			gbufferRenderView
		);
		f.gbuffer->build(gbufferRenderView, gbufferPass, entity);
		f.gbuffer->flush(gbufferRenderView, gbufferPass);

		f.haveGBuffer = true;
	}

	// Build shadow contexts.
	if (m_settings.shadowsEnabled)
		buildLightWithShadows(worldRenderView, entity, frame);
	else
		buildLightWithNoShadows(worldRenderView, entity, frame);

	// Build visual context.
	buildVisual(worldRenderView, entity, frame);

	m_count++;
}

bool WorldRendererPreLit::begin(int frame, render::EyeType eye, const Color4f& clearColor)
{
	if (!m_renderView->begin(m_visualTargetSet, 0))
		return false;

	m_renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);
	return true;
}

void WorldRendererPreLit::render(uint32_t flags, int frame, render::EyeType eye)
{
	Frame& f = m_frames[frame];
	Matrix44 projection;

	// Prepare stereoscopic projection.
	if (eye != render::EtCyclop)
	{
		float screenWidth = float(m_renderView->getWidth());
		
		float A = std::abs((c_distortionValue * c_interocularDistance) / screenWidth);
		float B = std::abs(A * c_screenPlaneDistance * (1.0f / f.projection(1, 1)));

		if (eye == render::EtLeft)
			A = -A;
		else
			B = -B;

		projection = translate(A, 0.0f, 0.0f) * f.projection * translate(B, 0.0f, 0.0f);
	}
	else
	{
		projection = f.projection;
	}

	// Prepare global program parameters.
	render::ProgramParameters programParams;
	programParams.beginParameters(m_globalContext);
	programParams.setMatrixParameter(ms_handleProjection, projection);
	programParams.endParameters(m_globalContext);

	// Render depth map; use as z-prepass if able to share depth buffer with primary.
	if ((flags & (WrfDepthMap | WrfNormalMap)) != 0)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Depth and normals");
		if (m_renderView->begin(m_gbufferTargetSet))
		{
			float farZ = m_settings.viewFarZ;

			const Color4f depthColor(farZ, farZ, farZ, farZ);
			const Color4f normalColor(0.5f, 0.5f, 0.0f, 0.5f);
			const Color4f clearColors[] = { depthColor, normalColor };

			m_renderView->clear(render::CfColor | render::CfDepth, clearColors, 1.0f, 0);

			if (f.haveGBuffer)
				f.gbuffer->getRenderContext()->render(m_renderView, render::RfOpaque, &programParams);

			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render shadow and light maps.
	if ((flags & (WrfShadowMap | WrfLightMap)) != 0 && f.lightCount > 0)
	{
		for (uint32_t i = 0; i < f.lightCount; ++i)
		{
			// Combine all shadow slices into a screen shadow mask.
			if ((flags & WrfShadowMap) != 0 && f.haveShadows[i])
			{
				for (int32_t j = 0; j < m_settings.shadowCascadingSlices; ++j)
				{
					T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
					if (m_renderView->begin(m_shadowTargetSet, 0))
					{
						const Color4f shadowClear(1.0f, 1.0f, 1.0f, 1.0f);
						m_renderView->clear(render::CfColor | render::CfDepth, &shadowClear, 1.0f, 0);
						f.slice[j].shadow[i]->getRenderContext()->render(m_renderView, render::RfOpaque, 0);
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
						Scalar zf(min(m_slicePositions[j + 1], m_settings.shadowFarZ));

						PostProcessStep::Instance::RenderParams params;
						params.viewFrustum = f.viewFrustum;
						params.viewToLight = f.slice[j].viewToLightSpace[i];
						params.projection = projection;
						params.squareProjection = f.slice[j].squareProjection[i];
						params.sliceNearZ = zn;
						params.sliceFarZ = zf;
						params.shadowFarZ = m_settings.shadowFarZ;
						params.shadowMapBias = m_settings.shadowMapBias + i * m_settings.shadowMapBiasCoeff;
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
				if (m_renderView->begin(m_shadowMaskFilterTargetSet[i], 0))
				{
					const Color4f maskClear(1.0f, 1.0f, 1.0f, 1.0f);
					m_renderView->clear(render::CfColor, &maskClear, 0.0f, 0);

					PostProcessStep::Instance::RenderParams params;
					params.viewFrustum = f.viewFrustum;
					params.projection = projection;
					params.sliceNearZ = 0.0f;
					params.sliceFarZ = m_settings.shadowFarZ;
					params.shadowMapBias = m_settings.shadowMapBias;
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
			}

			if ((flags & WrfLightMap) != 0)
			{
				T_RENDER_PUSH_MARKER(m_renderView, "World: Light primitive");
				if (m_renderView->begin(m_lightMapTargetSet, 0))
				{
					if (i == 0)
					{
						const Color4f lightClear(0.0f, 0.0f, 0.0f, 0.0f);
						m_renderView->clear(render::CfColor, &lightClear, 0.0f, 0);
					}
					m_lightRenderer->render(
						m_renderView,
						f.projection,
						f.view,
						f.lights[i],
						m_gbufferTargetSet->getColorTexture(0),
						m_gbufferTargetSet->getColorTexture(1),
						f.haveShadows[i] ? m_shadowMaskFilterTargetSet[i]->getWidth() : 0,
						f.haveShadows[i] ? m_shadowMaskFilterTargetSet[i]->getColorTexture(0) : 0
					);
					m_renderView->end();
				}
				T_RENDER_POP_MARKER(m_renderView);
			}
		}
	}
	else
	{
		// No active lights; ensure light map is cleared.
		if ((flags & WrfLightMap) != 0)
		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: Light map");
			if (m_renderView->begin(m_lightMapTargetSet, 0))
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
		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual opaque");
		f.visual->getRenderContext()->render(m_renderView, render::RfSetup | render::RfOpaque | render::RfPostOpaque, &programParams);
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
		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual alpha blend");
		f.visual->getRenderContext()->render(m_renderView, render::RfAlphaBlend | render::RfPostAlphaBlend | render::RfOverlay, &programParams);
		T_RENDER_POP_MARKER(m_renderView);
	}

	m_globalContext->flush();
}

void WorldRendererPreLit::end(int frame, render::EyeType eye, float deltaTime)
{
	Frame& f = m_frames[frame];

	world::PostProcessStep::Instance::RenderParams params;
	params.viewFrustum = f.viewFrustum;
	params.viewToLight = Matrix44::identity(); //f.viewToLightSpace;
	params.view = f.view;
	params.projection = f.projection;
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

PostProcess* WorldRendererPreLit::getVisualPostProcess()
{
	return m_visualPostProcess;
}

render::RenderTargetSet* WorldRendererPreLit::getVisualTargetSet()
{
	return m_visualTargetSet;
}

render::RenderTargetSet* WorldRendererPreLit::getDepthTargetSet()
{
	return m_gbufferTargetSet;
}

render::RenderTargetSet* WorldRendererPreLit::getShadowMaskTargetSet()
{
	return !m_shadowMaskFilterTargetSet.empty() ? m_shadowMaskFilterTargetSet[0].ptr() : 0;
}

void WorldRendererPreLit::getTargets(RefArray< render::ITexture >& outTargets) const
{
	outTargets.resize(4);
	outTargets[0] = m_gbufferTargetSet ? m_gbufferTargetSet->getColorTexture(0) : 0;
	outTargets[1] = m_gbufferTargetSet ? m_gbufferTargetSet->getColorTexture(1) : 0;
	outTargets[2] = m_lightMapTargetSet ? m_lightMapTargetSet->getColorTexture(0) : 0;
	outTargets[3] = !m_shadowMaskFilterTargetSet.empty() ? m_shadowMaskFilterTargetSet[0]->getColorTexture(0) : 0;
}

void WorldRendererPreLit::buildLightWithShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
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
			(light.type == LtDirectional || light.type == LtSpot) && light.castShadow
		)
		{
			for (int32_t slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
			{
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], m_settings.shadowFarZ));

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

				// Calculate shadow map projection.
				Matrix44 shadowLightView;
				Matrix44 shadowLightProjection;
				Matrix44 shadowLightSquareProjection = Matrix44::identity();
				Frustum shadowFrustum;

				m_shadowProjection->calculate(
					viewInverse,
					light.position,
					light.direction,
					sliceViewFrustum,
					shadowBox,
					m_settings.shadowQuantizeProjection,
					shadowLightView,
					shadowLightProjection,
					shadowLightSquareProjection,
					shadowFrustum
				);

				// Render shadow map.
				WorldRenderView shadowRenderView;
				shadowRenderView.resetLights();
				shadowRenderView.setProjection(shadowLightProjection);
				shadowRenderView.setSquareProjection(shadowLightSquareProjection);
				shadowRenderView.setView(shadowLightView);
				shadowRenderView.setViewFrustum(shadowFrustum);
				shadowRenderView.setCullFrustum(shadowFrustum);
				shadowRenderView.setTimes(
					worldRenderView.getTime(),
					worldRenderView.getDeltaTime(),
					worldRenderView.getInterval()
				);

				WorldRenderPassPreLit shadowPass(
					ms_techniqueShadow,
					shadowRenderView
				);
				f.slice[slice].shadow[i]->build(shadowRenderView, shadowPass, entity);
				f.slice[slice].shadow[i]->flush(shadowRenderView, shadowPass);
				
				f.slice[slice].viewToLightSpace[i] = shadowLightProjection * shadowLightView * viewInverse;
				f.slice[slice].squareProjection[i] = shadowLightSquareProjection;
			}

			f.haveShadows[i] = true;
		}
		else
			f.haveShadows[i] = false;
	}
}

void WorldRendererPreLit::buildLightWithNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];
	f.lightCount = worldRenderView.getLightCount();
	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);
		f.lights[i] = light;
	}
}

void WorldRendererPreLit::buildVisual(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();

	worldRenderView.resetLights();

	WorldRenderPassPreLit defaultPreLitPass(
		ms_techniquePreLitColor,
		worldRenderView,
		m_settings.fogEnabled,
		m_settings.fogDistance,
		m_settings.fogRange,
		m_fogColor,
		m_gbufferTargetSet->getColorTexture(0),
		m_gbufferTargetSet->getColorTexture(1),
		m_lightMapTargetSet->getColorTexture(0)
	);
	f.visual->build(worldRenderView, defaultPreLitPass, entity);
	f.visual->flush(worldRenderView, defaultPreLitPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
}

	}
}
