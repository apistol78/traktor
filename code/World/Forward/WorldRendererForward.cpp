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
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
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
render::handle_t s_handleReflectionMap = 0;

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
	s_handleReflectionMap = render::getParameterHandle(L"World_ReflectionMap");
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
	m_toneMapQuality = desc.toneMapQuality;
	m_shadowsQuality = desc.shadowsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;

	// Disable shadows early if projection is null.
	if (m_shadowSettings.maskProject.isNull())
		m_shadowsQuality = QuDisabled;

	m_frames.resize(desc.frameCount);

	// Create post process target pool to enable sharing of targets between multiple processes.
	Ref< render::ImageProcessTargetPool > postProcessTargetPool = new render::ImageProcessTargetPool(renderSystem);

	// Create "mini gbuffer" targets.
	{
		render::RenderTargetSetCreateDesc rtscd;

		rtscd.count = 2;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.targets[0].format = render::TfR16F;			// Depth (R)
		rtscd.targets[1].format = render::TfR16G16F;		// Normals (RG)

		m_gbufferTargetSet = renderSystem->createRenderTargetSet(rtscd);
		if (!m_gbufferTargetSet)
		{
			log::error << L"Unable to create depth render target." << Endl;
			return false;
		}
	}

	// Create projection and filter processes.
	resource::Proxy< render::ImageProcessSettings > shadowMaskProject;
	if (!resourceManager->bind(m_shadowSettings.maskProject, shadowMaskProject))
	{
		log::warning << L"Unable to create shadow project process; shadows disabled" << Endl;
		m_shadowsQuality = QuDisabled;
	}

	// Allocate "shadow map" targets.
	if (m_shadowsQuality > QuDisabled)
	{
		render::RenderSystemInformation info;
		renderSystem->getInformation(info);

		int32_t maxResolution = m_shadowSettings.resolution;
		if (info.dedicatedMemoryTotal < 512 * 1024 * 1024)
			maxResolution /= 2;

		int32_t resolution = min< int32_t >(nearestLog2(int32_t(max< int32_t >(desc.width, desc.height) * 1.9f)), maxResolution);
		T_DEBUG(L"Using shadow map resolution " << resolution);

		// Create shadow map target.
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 0;
		rtscd.width =
		rtscd.height = resolution;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		m_shadowTargetSet = renderSystem->createRenderTargetSet(rtscd);

		// Create shadow mask target.
		rtscd.count = 1;
		rtscd.width = desc.width / m_shadowSettings.maskDenominator;
		rtscd.height = desc.height / m_shadowSettings.maskDenominator;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.targets[0].format = render::TfR8;
		rtscd.preferTiled = true;
		m_shadowMaskProjectTargetSet = renderSystem->createRenderTargetSet(rtscd);

		m_shadowMaskProject = new render::ImageProcess();
		if (!m_shadowMaskProject->create(
			shadowMaskProject,
			postProcessTargetPool,
			resourceManager,
			renderSystem,
			rtscd.width,
			rtscd.height,
			desc.allTargetsPersistent
		))
		{
			log::warning << L"Unable to create shadow project process; shadows disabled" << Endl;
			m_shadowsQuality = QuDisabled;
		}

		// Use "shadow map is a depth texture" combination.
		m_shadowMaskProject->setCombination(render::getParameterHandle(L"World_ShadowMapDepthTexture"), true);

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

	// Create global reflection map.
	if (m_settings.reflectionMap)
	{
		if (!resourceManager->bind(m_settings.reflectionMap, m_reflectionMap))
			log::warning << L"Unable to create reflection map" << Endl;
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

	// Allocate "depth" context.
	for (auto& frame : m_frames)
		frame.depth = new WorldContext(desc.entityRenderers);

	// Allocate "shadow" contexts for each slice.
	if (m_shadowsQuality > QuDisabled)
	{
		for (auto& frame : m_frames)
		{
			for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
				frame.slice[i].shadow = new WorldContext(desc.entityRenderers);
		}
	}

	// Allocate "visual" contexts.
	for (auto& frame : m_frames)
		frame.visual = new WorldContext(desc.entityRenderers);

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
	for (auto& frame : m_frames)
	{
		for (int32_t i = 0; i < MaxSliceCount; ++i)
			frame.slice[i].shadow = nullptr;

		frame.visual = nullptr;
		frame.depth = nullptr;
	}

	safeDestroy(m_toneMapImageProcess);
	safeDestroy(m_gammaCorrectionImageProcess);
	safeDestroy(m_visualImageProcess);
	safeDestroy(m_shadowMaskProject);
	m_reflectionMap.clear();
	safeDestroy(m_shadowMaskProjectTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_gbufferTargetSet);
	safeDestroy(m_visualTargetSet);

	m_renderView = nullptr;
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
	{
		f.depth->getRenderContext()->flush();
		f.haveDepth = false;
	}

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

	// Build depth context.
	buildGBuffer(worldRenderView, frame);

	if (m_shadowsQuality > QuDisabled)
		buildShadows(worldRenderView, frame);
	else
		buildNoShadows(worldRenderView, frame);

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

void WorldRendererForward::render(int frame, render::EyeType eye)
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

	// Render gbuffer.
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: GBuffer");
		if (m_renderView->begin(m_gbufferTargetSet))
		{
			const float clearZ = f.viewFrustum.getFarZ();
			const Color4f depthColor(clearZ, clearZ, clearZ, clearZ);
			const Color4f normalColor(0.0f, 0.0f, 1.0f, 0.0f);
			const Color4f clearColors[] = { depthColor, normalColor };

			m_renderView->clear(render::CfColor | render::CfDepth, clearColors, 1.0f, 0);
			f.depth->getRenderContext()->render(m_renderView, render::RpOpaque, &defaultProgramParams);
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render shadow map.
	if (eye == render::EtCyclop || eye == render::EtLeft)
	{
		if (f.haveShadows)
		{
			for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
			{
				render::ProgramParameters shadowProgramParams;
				shadowProgramParams.beginParameters(m_globalContext);
				shadowProgramParams.setFloatParameter(s_handleTime, f.time);
				shadowProgramParams.setMatrixParameter(s_handleView, f.slice[i].shadowLightView);
				shadowProgramParams.setMatrixParameter(s_handleViewInverse, f.slice[i].shadowLightView.inverse());
				shadowProgramParams.setMatrixParameter(s_handleProjection, f.slice[i].shadowLightProjection);
				shadowProgramParams.endParameters(m_globalContext);

				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
				if (m_renderView->begin(m_shadowTargetSet))
				{
					m_renderView->clear(render::CfDepth, nullptr, 1.0f, 0);
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

					render::ImageProcessStep::Instance::RenderParams params;
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
						m_shadowTargetSet->getDepthTexture(),	// color
						m_gbufferTargetSet->getColorTexture(0),	// depth
						m_gbufferTargetSet->getColorTexture(1),	// normal
						nullptr,	// velocity
						nullptr,	// shadow mask
						params
					);

					m_renderView->end();
				}
				T_RENDER_POP_MARKER(m_renderView);
			}
		}
	}

	// Render visuals.
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual");
		f.visual->getRenderContext()->render(m_renderView, render::RpAll, &defaultProgramParams);
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

		render::ImageProcessStep::Instance::RenderParams params;
		params.viewFrustum = f.viewFrustum;
		params.viewToLight = f.viewToLightSpace;
		params.view = f.view;
		params.projection = f.projection;
		// params.godRayDirection = f.godRayDirection;
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
			T_RENDER_PUSH_MARKER(m_renderView, "World: Post process");

			bool haveNext = bool((i + 1) < processes.size());
			if (haveNext)
				m_renderView->begin(outputTargetSet);

			processes[i]->render(
				m_renderView,
				sourceTargetSet->getColorTexture(0),	// color
				m_gbufferTargetSet->getColorTexture(0),		// depth
				m_gbufferTargetSet->getColorTexture(1),		// normal
				nullptr,	// velocity
				m_shadowTargetSet ? m_shadowTargetSet->getColorTexture(0) : nullptr,	// shadow mask
				params
			);

			if (haveNext)
			{
				m_renderView->end();
				std::swap(sourceTargetSet, outputTargetSet);
			}

			T_RENDER_POP_MARKER(m_renderView);
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
	}

	if (m_shadowTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow map (last cascade)", render::DtvShadowMap, m_shadowTargetSet->getColorTexture(0)));

	if (m_shadowMaskProjectTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow mask (projection)",render:: DtvShadowMask, m_shadowMaskProjectTargetSet->getColorTexture(0)));

	if (m_shadowMaskProject)
		m_shadowMaskProject->getDebugTargets(outTargets);

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

void WorldRendererForward::buildGBuffer(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];

	WorldRenderPassForward pass(
		s_techniqueForwardGBufferWrite,
		worldRenderView,
		IWorldRenderPass::PfFirst,
		nullptr,
		nullptr
	);
	for (auto entity : m_buildEntities)
		f.depth->build(worldRenderView, pass, entity);
	f.depth->flush(worldRenderView, pass);

	f.haveDepth = true;
}

void WorldRendererForward::buildShadows(WorldRenderView& worldRenderView, int frame)
{
	// Find first directional light casting shadow.
	const Light* shadowLight = 0;
	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
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
		buildNoShadows(worldRenderView, frame);
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
			shadowFrustum
		);

		// Render shadow map.
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
			shadowRenderView,
			IWorldRenderPass::PfNone,
			nullptr,
			nullptr
		);
		for (auto entity : m_buildEntities)
			f.slice[slice].shadow->build(shadowRenderView, shadowPass, entity);
		f.slice[slice].shadow->flush(shadowRenderView, shadowPass);

		f.slice[slice].shadowLightView = shadowLightView;
		f.slice[slice].shadowLightProjection = shadowLightProjection;
		f.slice[slice].viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
	}

	worldRenderView.resetLights();

	// Render visuals.
	WorldRenderPassForward defaultPass(
		s_techniqueForwardColor,
		worldRenderView,
		IWorldRenderPass::PfLast,
		m_settings.ambientColor,
		m_settings.fog,
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ,
		m_settings.fogColor,
		nullptr,
		m_gbufferTargetSet->getColorTexture(0),
		m_shadowMaskProjectTargetSet->getColorTexture(0)
	);
	for (auto entity : m_buildEntities)
		f.visual->build(worldRenderView, defaultPass, entity);
	f.visual->flush(worldRenderView, defaultPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.haveShadows = true;
}

void WorldRendererForward::buildNoShadows(WorldRenderView& worldRenderView, int frame)
{
	Frame& f = m_frames[frame];

	worldRenderView.resetLights();

	WorldRenderPassForward defaultPass(
		s_techniqueForwardColor,
		worldRenderView,
		IWorldRenderPass::PfLast,
		m_settings.ambientColor,
		m_settings.fog,
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ,
		m_settings.fogColor,
		nullptr,
		m_gbufferTargetSet->getColorTexture(0),
		nullptr
	);
	for (auto entity : m_buildEntities)
		f.visual->build(worldRenderView, defaultPass, entity);
	f.visual->flush(worldRenderView, defaultPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.haveShadows = false;
}

	}
}
