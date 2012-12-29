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
#include "Resource/Id.h"
#include "Resource/IResourceManager.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/Entity.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
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

render::handle_t s_techniqueDefault = 0;
render::handle_t s_techniqueDepth = 0;
render::handle_t s_techniqueShadow = 0;
render::handle_t s_handleProjection = 0;

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
	s_handleProjection = render::getParameterHandle(L"Projection");
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

	m_frames.resize(desc.frameCount);

	float fogColor[4];
	m_settings.fogColor.getRGBA32F(fogColor);
	m_fogColor = Vector4::loadUnaligned(fogColor);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create "unprocessed" target.
	{
		render::RenderTargetSetCreateDesc rtscd;
		
		rtscd.count = 1;
		rtscd.width = width;
		rtscd.height = height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = true;
		rtscd.preferTiled = true;
		rtscd.targets[0].format = render::TfR8G8B8A8;

		if (desc.postProcessSettings && desc.postProcessSettings->requireHighRange())
			rtscd.targets[0].format = render::TfR11G11B10F;

		m_visualTargetSet = renderSystem->createRenderTargetSet(rtscd);
		if (!m_visualTargetSet)
			return false;
	}

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
		render::RenderTargetSetCreateDesc rtscd;

		// Create shadow map target.
		rtscd.count = 1;
		rtscd.width =
		rtscd.height = m_shadowSettings.resolution;
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
				m_shadowProjection = new UniformShadowProjection(m_shadowSettings.resolution);
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

	// Create "visual" post processing filter.
	if (desc.postProcessSettings)
	{
		m_visualPostProcess = new world::PostProcess();
		if (!m_visualPostProcess->create(
			desc.postProcessSettings,
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

	safeDestroy(m_visualPostProcess);
	safeDestroy(m_shadowMaskFilter);
	safeDestroy(m_shadowMaskProject);
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

void WorldRendererForward::build(WorldRenderView& worldRenderView, Entity* entity, int frame)
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

	if (m_settings.depthPassEnabled || m_shadowsQuality > QuDisabled)
	{
		WorldRenderView depthRenderView = worldRenderView;
		depthRenderView.resetLights();

		WorldRenderPassForward pass(
			s_techniqueDepth,
			depthRenderView,
			0
		);
		f.depth->build(depthRenderView, pass, entity);
		f.depth->flush(depthRenderView, pass);

		f.haveDepth = true;
	}
	else
		f.haveDepth = false;

	if (m_shadowsQuality > QuDisabled)
		buildShadows(worldRenderView, entity, frame);
	else
		buildNoShadows(worldRenderView, entity, frame);

	// Prepare stereoscopic projection.
	float screenWidth = float(m_renderView->getWidth());
	f.A = std::abs((worldRenderView.getDistortionValue() * worldRenderView.getInterocularDistance()) / screenWidth);
	f.B = std::abs(f.A * worldRenderView.getScreenPlaneDistance() * (1.0f / f.projection(1, 1)));

	m_count++;
}

bool WorldRendererForward::begin(int frame, render::EyeType eye, const Color4f& clearColor)
{
	if (m_visualTargetSet)
	{
		if (!m_renderView->begin(m_visualTargetSet, 0))
			return false;

		m_renderView->clear(render::CfColor | render::CfDepth, &clearColor, 1.0f, 0);
	}
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
	render::ProgramParameters programParams;
	programParams.beginParameters(m_globalContext);
	programParams.setMatrixParameter(s_handleProjection, projection);
	programParams.endParameters(m_globalContext);

	// Render depth map; use as z-prepass if able to share depth buffer with primary.
	if ((flags & WrfDepthMap) != 0 && f.haveDepth)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Depth");
		if (m_renderView->begin(m_depthTargetSet, 0))
		{
			float farZ = m_settings.viewFarZ;
			const Color4f depthColor(farZ, farZ, farZ, farZ);
			m_renderView->clear(render::CfColor | render::CfDepth, &depthColor, 1.0f, 0);
			f.depth->getRenderContext()->render(m_renderView, render::RfOpaque, &programParams);
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
				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
				if (m_renderView->begin(m_shadowTargetSet, 0))
				{
					const Color4f shadowClear(1.0f, 1.0f, 1.0f, 1.0f);
					m_renderView->clear(render::CfColor | render::CfDepth, &shadowClear, 1.0f, 0);
					f.slice[i].shadow->getRenderContext()->render(m_renderView, render::RfOpaque, 0);
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
					params.squareProjection = f.slice[i].squareProjection;
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
		uint32_t renderFlags = render::RfSetup | render::RfOverlay;

		if (flags & WrfVisualOpaque)
			renderFlags |= render::RfOpaque | render::RfPostOpaque;
		if (flags & WrfVisualAlphaBlend)
			renderFlags |= render::RfAlphaBlend | render::RfPostAlphaBlend;

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual");
		f.visual->getRenderContext()->render(m_renderView, renderFlags, &programParams);
		T_RENDER_POP_MARKER(m_renderView);
	}

	m_globalContext->flush();
}

void WorldRendererForward::end(int frame, render::EyeType eye, float deltaTime)
{
	if (m_visualTargetSet)
	{
		m_renderView->end();

		if (m_visualPostProcess)
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

			world::PostProcessStep::Instance::RenderParams params;
			params.viewFrustum = f.viewFrustum;
			params.viewToLight = f.viewToLightSpace;
			params.view = f.view;
			params.projection = projection;
			params.deltaTime = deltaTime;

			m_visualPostProcess->render(
				m_renderView,
				m_visualTargetSet,
				m_depthTargetSet,
				m_shadowTargetSet,
				params
			);
		}
		else
		{
			// \FIXME: Plain blit to primary target.
		}
	}
}

PostProcess* WorldRendererForward::getVisualPostProcess()
{
	return m_visualPostProcess;
}

render::RenderTargetSet* WorldRendererForward::getVisualTargetSet()
{
	return m_visualTargetSet;
}

render::RenderTargetSet* WorldRendererForward::getDepthTargetSet()
{
	return m_depthTargetSet;
}

render::RenderTargetSet* WorldRendererForward::getShadowMaskTargetSet()
{
	return m_shadowMaskFilterTargetSet;
}

void WorldRendererForward::getTargets(RefArray< render::ITexture >& outTargets) const
{
	outTargets.resize(4);
	outTargets[0] = m_depthTargetSet ? m_depthTargetSet->getColorTexture(0) : 0;
	outTargets[1] = m_shadowTargetSet ? m_shadowTargetSet->getColorTexture(0) : 0;
	outTargets[2] = m_shadowMaskProjectTargetSet ? m_shadowMaskProjectTargetSet->getColorTexture(0) : 0;
	outTargets[3] = m_shadowMaskFilterTargetSet ? m_shadowMaskFilterTargetSet->getColorTexture(0) : 0;
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
	Vector4 eyePosition = viewInverse.translation();

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
		shadowRenderView.setSquareProjection(shadowLightSquareProjection);
		shadowRenderView.setView(shadowLightView);
		shadowRenderView.setViewFrustum(shadowFrustum);
		shadowRenderView.setCullFrustum(shadowFrustum);
		shadowRenderView.setTimes(
			worldRenderView.getTime(),
			worldRenderView.getDeltaTime(),
			worldRenderView.getInterval()
		);

		WorldRenderPassForward shadowPass(
			s_techniqueShadow,
			shadowRenderView,
			0
		);
		f.slice[slice].shadow->build(shadowRenderView, shadowPass, entity);
		f.slice[slice].shadow->flush(shadowRenderView, shadowPass);
				
		f.slice[slice].viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
		f.slice[slice].squareProjection = shadowLightSquareProjection;
	}

	// Render visuals.
	worldRenderView.resetLights();

	WorldRenderPassForward defaultPass(
		s_techniqueDefault,
		worldRenderView,
		m_settings.fogEnabled,
		m_settings.fogDistance,
		m_settings.fogRange,
		m_fogColor,
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

	worldRenderView.resetLights();

	WorldRenderPassForward defaultPass(
		s_techniqueDefault,
		worldRenderView,
		m_settings.fogEnabled,
		m_settings.fogDistance,
		m_settings.fogRange,
		m_fogColor,
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
