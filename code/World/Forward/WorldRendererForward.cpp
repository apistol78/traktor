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
#include "Resource/Id.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessTargetPool.h"
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

const resource::Id< PostProcessSettings > c_ambientOcclusionLow(Guid(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}"));		// SSAO, half size
const resource::Id< PostProcessSettings > c_ambientOcclusionMedium(Guid(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}"));	// SSAO, full size
const resource::Id< PostProcessSettings > c_ambientOcclusionHigh(Guid(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}"));		// HBAO, half size
const resource::Id< PostProcessSettings > c_ambientOcclusionUltra(Guid(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}"));	// HBAO, full size

const resource::Id< PostProcessSettings > c_antiAliasNone(Guid(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}"));
const resource::Id< PostProcessSettings > c_antiAliasLow(Guid(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}"));
const resource::Id< PostProcessSettings > c_antiAliasMedium(Guid(L"{3E1D810B-339A-F742-9345-4ECA00220D57}"));
const resource::Id< PostProcessSettings > c_antiAliasHigh(Guid(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}"));
const resource::Id< PostProcessSettings > c_antiAliasUltra(Guid(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}"));

const resource::Id< PostProcessSettings > c_gammaCorrection(Guid(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}"));

render::handle_t s_techniqueDefault = 0;
render::handle_t s_techniqueDepth = 0;
render::handle_t s_techniqueShadow = 0;
render::handle_t s_handleTime = 0;
render::handle_t s_handleProjection = 0;
render::handle_t s_handleSquareProjection = 0;
render::handle_t s_handleReflectionMap = 0;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererForward", 0, WorldRendererForward, IWorldRenderer)

WorldRendererForward::WorldRendererForward()
:	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
,	m_count(0)
{
	// Techniques
	s_techniqueDefault = render::getParameterHandle(L"World_ForwardColor");
	s_techniqueDepth = render::getParameterHandle(L"World_DepthWrite");
	s_techniqueShadow = render::getParameterHandle(L"World_ShadowWrite");

	// Global parameters.
	s_handleTime = render::getParameterHandle(L"Time");
	s_handleProjection = render::getParameterHandle(L"Projection");
	s_handleSquareProjection = render::getParameterHandle(L"SquareProjection");
	s_handleReflectionMap = render::getParameterHandle(L"ReflectionMap");
}

bool WorldRendererForward::create(
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

	// Disable shadows early if filter or projection is null.
	if (m_shadowSettings.maskFilter.isNull() || m_shadowSettings.maskProject.isNull())
		m_shadowsQuality = QuDisabled;

	m_frames.resize(desc.frameCount);

	float fogColor[4];
	m_settings.fogColor.getRGBA32F(fogColor);
	m_fogColor = Vector4::loadUnaligned(fogColor);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create post process target pool to enable sharing of targets between multiple processes.
	Ref< PostProcessTargetPool > postProcessTargetPool = new PostProcessTargetPool(renderSystem);

	// Create "depth map" target.
	if (m_settings.depthPassEnabled || m_shadowsQuality > QuDisabled)
	{
		render::RenderTargetSetCreateDesc rtscd;

		rtscd.count = 1;
		rtscd.width = width;
		rtscd.height = height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = true;
		rtscd.preferTiled = true;
		rtscd.targets[0].format = render::TfR16F;

		m_depthTargetSet = renderSystem->createRenderTargetSet(rtscd);

		if (!m_depthTargetSet && desc.multiSample > 0)
		{
			rtscd.multiSample = 0;
			rtscd.createDepthStencil = true;
			rtscd.usingPrimaryDepthStencil = false;

			m_depthTargetSet = renderSystem->createRenderTargetSet(rtscd);
			if (m_depthTargetSet)
				log::warning << L"MSAA depth render target unsupported; may cause poor performance" << Endl;
		}

		if (!m_depthTargetSet)
		{
			log::warning << L"Unable to create depth render target; depth disabled" << Endl;
			m_settings.depthPassEnabled = false;
		}
	}

	// Allocate "shadow map" targets.
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
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width =
		rtscd.height = resolution;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.preferTiled = true;
		rtscd.targets[0].format = render::TfR16F;
		m_shadowTargetSet = renderSystem->createRenderTargetSet(rtscd);

		// Create shadow mask target.
		rtscd.count = 1;
		rtscd.width = width / m_shadowSettings.maskDenominator;
		rtscd.height = height / m_shadowSettings.maskDenominator;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.targets[0].format = render::TfR8;
		rtscd.preferTiled = true;
		m_shadowMaskProjectTargetSet = renderSystem->createRenderTargetSet(rtscd);

		// Create filtered shadow mask target.
		rtscd.count = 1;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.targets[0].format = render::TfR8;
		rtscd.preferTiled = true;
		m_shadowMaskFilterTargetSet = renderSystem->createRenderTargetSet(rtscd);

		if (
			m_shadowTargetSet &&
			m_shadowMaskProjectTargetSet &&
			m_shadowMaskFilterTargetSet
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
					rtscd.width,
					rtscd.height
				))
				{
					log::warning << L"Unable to create shadow project process; shadows disabled (2)" << Endl;
					m_shadowsQuality = QuDisabled;
				}

				m_shadowMaskFilter = new PostProcess();
				if (!m_shadowMaskFilter->create(
					shadowMaskFilter,
					postProcessTargetPool,
					resourceManager,
					renderSystem,
					rtscd.width,
					rtscd.height
				))
				{
					log::warning << L"Unable to create shadow filter process; shadows disabled (3)" << Endl;
					m_shadowsQuality = QuDisabled;
				}
			}
		}
		else
		{
			log::warning << L"Unable to create shadow render targets; shadows disabled (4)" << Endl;
			m_shadowsQuality = QuDisabled;
		}

		if (m_shadowsQuality > QuDisabled)
		{
			switch (m_shadowSettings.projection)
			{
			case WorldRenderSettings::SpBox:
				m_shadowProjection = new BoxShadowProjection();
				break;

			case WorldRenderSettings::SpLiSP:
				m_shadowProjection = new LiSPShadowProjection();
				break;

			case WorldRenderSettings::SpTrapezoid:
				m_shadowProjection = new TrapezoidShadowProjection();
				break;

			default:
			case WorldRenderSettings::SpUniform:
				m_shadowProjection = new UniformShadowProjection(resolution);
				break;
			}
		}

		// Ensure targets are destroyed if something went wrong in setup.
		if (m_shadowsQuality == QuDisabled)
		{
			safeDestroy(m_shadowTargetSet);
			safeDestroy(m_shadowMaskProjectTargetSet);
			safeDestroy(m_shadowMaskFilterTargetSet);
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
	if (m_antiAlias || m_visualPostProcess || m_gammaCorrectionPostProcess)
	{
		render::RenderTargetSetCreateDesc rtscd;
		
		rtscd.count = 1;
		rtscd.width = width;
		rtscd.height = height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = true;
		rtscd.preferTiled = true;

		if (m_settings.linearLighting || (m_visualPostProcess && m_visualPostProcess->requireHighRange()))
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

	// Allocate "depth" context.
	if (m_settings.depthPassEnabled || m_shadowsQuality > QuDisabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->depth = new WorldContext(desc.entityRenderers, 0);
	}

	// Allocate "shadow" contexts for each slice.
	if (m_shadowsQuality > QuDisabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		{
			for (int32_t j = 0; j < m_shadowSettings.cascadingSlices; ++j)
				i->slice[j].shadow = new WorldContext(desc.entityRenderers, 0);
		}
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(desc.entityRenderers, 0);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

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

void WorldRendererForward::destroy()
{
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		for (int32_t j = 0; j < MaxSliceCount; ++j)
			i->slice[j].shadow = 0;

		i->visual = 0;
		i->depth = 0;
	}

	safeDestroy(m_gammaCorrectionPostProcess);
	safeDestroy(m_visualPostProcess);
	safeDestroy(m_shadowMaskFilter);
	safeDestroy(m_shadowMaskProject);
	m_reflectionMap.clear();
	safeDestroy(m_shadowMaskFilterTargetSet);
	safeDestroy(m_shadowMaskProjectTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_depthTargetSet);
	safeDestroy(m_visualTargetSet);

	m_renderView = 0;
}

void WorldRendererForward::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
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

void WorldRendererForward::createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const
{
	float viewFarZ = m_settings.viewFarZ;

	Frustum viewFrustum;
	viewFrustum.buildOrtho(worldView.width, worldView.height, -viewFarZ, viewFarZ);

	outRenderView.setViewSize(Vector2(worldView.width, worldView.height));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(orthoLh(worldView.width, worldView.height, -viewFarZ, viewFarZ));
}

bool WorldRendererForward::beginBuild()
{
	m_buildEntities.clear();
	return true;
}

void WorldRendererForward::build(Entity* entity)
{
	m_buildEntities.push_back(entity);
}

void WorldRendererForward::endBuild(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];

	if (f.haveDepth)
		f.depth->getRenderContext()->flush();

	if (f.haveShadows)
	{
		int32_t cascadingSlices = m_settings.shadowSettings[m_shadowsQuality].cascadingSlices;
		for (int32_t i = 0; i < cascadingSlices; ++i)
			f.slice[i].shadow->getRenderContext()->flush();
	}

	f.visual->getRenderContext()->flush();

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	worldRenderView.setEyePosition(viewInverse.translation().xyz1());
	worldRenderView.setEyeDirection(viewInverse.axisZ().xyz0());

	// Store some global values.
	f.time = worldRenderView.getTime();

	if (m_settings.depthPassEnabled || m_shadowsQuality > QuDisabled)
	{
		WorldRenderView depthRenderView = worldRenderView;
		WorldRenderPassForward pass(
			s_techniqueDepth,
			depthRenderView,
			0,
			0
		);
		for (RefArray< Entity >::const_iterator i = m_buildEntities.begin(); i != m_buildEntities.end(); ++i)
			f.depth->build(depthRenderView, pass, *i);
		f.depth->flush(depthRenderView, pass);

		f.haveDepth = true;
	}
	else
		f.haveDepth = false;

	if (m_shadowsQuality > QuDisabled)
	{
		for (RefArray< Entity >::const_iterator i = m_buildEntities.begin(); i != m_buildEntities.end(); ++i)
			buildShadows(worldRenderView, *i, frame);
	}
	else
	{
		for (RefArray< Entity >::const_iterator i = m_buildEntities.begin(); i != m_buildEntities.end(); ++i)
			buildNoShadows(worldRenderView, *i, frame);
	}

	worldRenderView.resetLights();

	// Prepare stereoscopic projection.
	float screenWidth = float(m_renderView->getWidth());
	f.A = std::abs((worldRenderView.getDistortionValue() * worldRenderView.getInterocularDistance()) / screenWidth);
	f.B = std::abs(f.A * worldRenderView.getScreenPlaneDistance() * (1.0f / f.projection(1, 1)));

	m_count++;
}

bool WorldRendererForward::beginRender(int frame, render::EyeType eye, const Color4f& clearColor)
{
	if (m_visualTargetSet)
	{
		if (!m_renderView->begin(m_visualTargetSet, 0))
			return false;
	}

	m_renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);
	return true;
}

void WorldRendererForward::render(uint32_t flags, int frame, render::EyeType eye)
{
	Frame& f = m_frames[frame];
	Matrix44 projection;

	// Calculate stereoscopic projection.
	if (eye != render::EtCyclop)
	{
		float A = f.A;
		float B = f.B;

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
	render::ProgramParameters defaultProgramParams;
	defaultProgramParams.beginParameters(m_globalContext);
	defaultProgramParams.setFloatParameter(s_handleTime, f.time);
	defaultProgramParams.setMatrixParameter(s_handleProjection, projection);
	defaultProgramParams.setTextureParameter(s_handleReflectionMap, m_reflectionMap);
	defaultProgramParams.endParameters(m_globalContext);

	// Render depth map; use as z-prepass if able to share depth buffer with primary.
	if ((flags & WrfDepthMap) != 0 && f.haveDepth)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Depth");
		if (m_renderView->begin(m_depthTargetSet, 0))
		{
			float farZ = m_settings.viewFarZ;
			const Color4f depthColor(farZ, farZ, farZ, farZ);
			m_renderView->clear(render::CfColor | render::CfDepth, &depthColor, 1.0f, 0);
			f.depth->getRenderContext()->render(m_renderView, render::RpOpaque, &defaultProgramParams);
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}
	else if (!f.haveDepth)
	{
		// No depth pass; ensure primary depth is cleared.
		const Color4f nullColor(0.0f, 0.0f, 0.0f, 0.0f);
		m_renderView->clear(render::CfDepth, &nullColor, 1.0f, 0);
	}

	// Render shadow map.
	if (eye == render::EtCyclop || eye == render::EtLeft)
	{
		if ((flags & WrfShadowMap) != 0 && f.haveShadows)
		{
			for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
			{
				render::ProgramParameters shadowProgramParams;
				shadowProgramParams.beginParameters(m_globalContext);
				shadowProgramParams.setFloatParameter(s_handleTime, f.time);
				shadowProgramParams.setMatrixParameter(s_handleProjection, f.slice[i].shadowLightProjection);
				shadowProgramParams.setMatrixParameter(s_handleSquareProjection, f.slice[i].shadowLightSquareProjection);
				shadowProgramParams.endParameters(m_globalContext);

				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
				if (m_renderView->begin(m_shadowTargetSet, 0))
				{
					const Color4f shadowClear(1.0f, 1.0f, 1.0f, 1.0f);
					m_renderView->clear(render::CfColor | render::CfDepth, &shadowClear, 1.0f, 0);
					f.slice[i].shadow->getRenderContext()->render(m_renderView, render::RpOpaque, &shadowProgramParams);
					m_renderView->end();
				}
				T_RENDER_POP_MARKER(m_renderView);

				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask project");
				if (m_renderView->begin(m_shadowMaskProjectTargetSet, 0))
				{
					if (i == 0)
					{
						const Color4f maskClear(1.0f, 1.0f, 1.0f, 1.0f);
						m_renderView->clear(render::CfColor, &maskClear, 0.0f, 0);
					}

					Scalar zn(max(m_slicePositions[i], m_settings.viewNearZ));
					Scalar zf(min(m_slicePositions[i + 1], m_shadowSettings.farZ));

					PostProcessStep::Instance::RenderParams params;
					params.viewFrustum = f.viewFrustum;
					params.viewToLight = f.slice[i].viewToLightSpace;
					params.projection = projection;
					params.sliceCount = m_shadowSettings.cascadingSlices;
					params.sliceIndex = i;
					params.sliceNearZ = zn;
					params.sliceFarZ = zf;
					params.shadowFarZ = m_shadowSettings.farZ;
					params.shadowMapBias = m_shadowSettings.bias + i * m_shadowSettings.biasCoeff;
					params.deltaTime = 0.0f;

					m_shadowMaskProject->render(
						m_renderView,
						m_shadowTargetSet,
						m_depthTargetSet,
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
					m_depthTargetSet,
					0,
					params
				);
				m_renderView->end();
			}
			T_RENDER_POP_MARKER(m_renderView);
		}
	}

	// Render visuals.
	if ((flags & (WrfVisualOpaque | WrfVisualAlphaBlend)) != 0)
	{
		uint32_t renderFlags = render::RpSetup | render::RpOverlay;

		if (flags & WrfVisualOpaque)
			renderFlags |= render::RpOpaque | render::RpPostOpaque;
		if (flags & WrfVisualAlphaBlend)
			renderFlags |= render::RpAlphaBlend | render::RpPostAlphaBlend;

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual");
		f.visual->getRenderContext()->render(m_renderView, renderFlags, &defaultProgramParams);
		T_RENDER_POP_MARKER(m_renderView);
	}

	m_globalContext->flush();
}

void WorldRendererForward::endRender(int frame, render::EyeType eye, float deltaTime)
{
	Frame& f = m_frames[frame];
	Matrix44 projection;

	// Calculate stereoscopic projection.
	if (eye != render::EtCyclop)
	{
		float A = f.A;
		float B = f.B;

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

	if (m_visualTargetSet)
	{
		m_renderView->end();
	
		render::RenderTargetSet* sourceTargetSet = m_visualTargetSet;
		render::RenderTargetSet* outputTargetSet = m_intermediateTargetSet;

		world::PostProcessStep::Instance::RenderParams params;
		params.viewFrustum = f.viewFrustum;
		params.viewToLight = f.viewToLightSpace;
		params.view = f.view;
		params.projection = f.projection;
		params.deltaTime = deltaTime;

		// Apply custom post processing filter.
		if (m_visualPostProcess)
		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: Custom PP");
	
			if (m_gammaCorrectionPostProcess || m_antiAlias)
				m_renderView->begin(outputTargetSet);
	
			m_visualPostProcess->render(
				m_renderView,
				sourceTargetSet,
				m_depthTargetSet,
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
				m_depthTargetSet,
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
				m_depthTargetSet,
				m_shadowTargetSet,
				params
			);
	
			T_RENDER_POP_MARKER(m_renderView);
		}
	}
}

PostProcess* WorldRendererForward::getVisualPostProcess()
{
	return m_visualPostProcess;
}

void WorldRendererForward::getDebugTargets(std::vector< DebugTarget >& outTargets) const
{
	if (m_depthTargetSet)
		outTargets.push_back(DebugTarget(L"View depth", DtvDepth, m_depthTargetSet->getColorTexture(0)));

	if (m_shadowTargetSet)
		outTargets.push_back(DebugTarget(L"Shadow map (last cascade)", DtvShadowMap, m_shadowTargetSet->getDepthTexture()));
	
	if (m_shadowMaskProjectTargetSet)
		outTargets.push_back(DebugTarget(L"Shadow mask (projection)", DtvShadowMask, m_shadowMaskProjectTargetSet->getDepthTexture()));

	if (m_shadowMaskFilterTargetSet)
		outTargets.push_back(DebugTarget(L"Shadow mask (SS filtered)", DtvShadowMask, m_shadowMaskFilterTargetSet->getDepthTexture()));
}

void WorldRendererForward::buildShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	// Find first directional light casting shadow.
	const Light* shadowLight = 0;
	for (int32_t i = 0; i < MaxLightCount; ++i)
	{
		const Light& light = worldRenderView.getLight(i);
		if (light.type == LtDirectional && light.castShadow)
		{
			shadowLight = &light;
			break;
		}
	}

	// If no shadow casting light found, we do simple path.
	if (!shadowLight)
	{
		buildNoShadows(worldRenderView, entity, frame);
		return;
	}

	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();

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

		m_shadowProjection->calculate(
			viewInverse,
			shadowLight->position,
			shadowLight->direction,
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

		WorldRenderPassForward shadowPass(
			s_techniqueShadow,
			shadowRenderView,
			0,
			0
		);
		f.slice[slice].shadow->build(shadowRenderView, shadowPass, entity);
		f.slice[slice].shadow->flush(shadowRenderView, shadowPass);
		
		f.slice[slice].shadowLightProjection = shadowLightProjection;
		f.slice[slice].shadowLightSquareProjection = shadowLightSquareProjection;
		f.slice[slice].viewToLightSpace = shadowLightSquareProjection * shadowLightProjection * shadowLightView * viewInverse;
	}

	// Render visuals.
	WorldRenderPassForward defaultPass(
		s_techniqueDefault,
		worldRenderView,
		m_settings.fogEnabled,
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ,
		m_fogColor,
		0,
		f.haveDepth ? m_depthTargetSet->getColorTexture(0) : 0,
		m_shadowMaskFilterTargetSet->getColorTexture(0)
	);
	f.visual->build(worldRenderView, defaultPass, entity);
	f.visual->flush(worldRenderView, defaultPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.haveShadows = true;
}

void WorldRendererForward::buildNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	WorldRenderPassForward defaultPass(
		s_techniqueDefault,
		worldRenderView,
		m_settings.fogEnabled,
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ,
		m_fogColor,
		0,
		f.haveDepth ? m_depthTargetSet->getColorTexture(0) : 0,
		0
	);
	f.visual->build(worldRenderView, defaultPass, entity);
	f.visual->flush(worldRenderView, defaultPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.haveShadows = false;
}

	}
}
