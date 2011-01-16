#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/Entity.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PreLit/WorldRendererPreLit.h"
#include "World/PreLit/WorldRenderPassPreLit.h"
#include "World/SMProj/BoxSMProj.h"
#include "World/SMProj/LiSPSMProj.h"
#include "World/SMProj/TSMProj.h"
#include "World/SMProj/UniformSMProj.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const Guid c_shadowMaskProjectionSettingsNoFilter(L"{19222311-363F-CB45-86E5-34D376CDA8AD}");
const Guid c_shadowMaskProjectionSettingsLow(L"{7D4D38B9-1E43-8046-B1A4-705CFEF9B8EB}");
const Guid c_shadowMaskProjectionSettingsMedium(L"{57FD53AF-547A-9F46-8C94-B4D24EFB63BC}");
const Guid c_shadowMaskProjectionSettingsHigh(L"{FABC4017-4D65-604D-B9AB-9FC03FE3CE43}");
const Guid c_shadowMaskProjectionSettingsHighest(L"{5AFC153E-6FCE-3142-9E1B-DD3722DA447F}");

const static float c_interocularDistance = 6.5f;
const static float c_distortionValue = 0.8f;
const static float c_screenPlaneDistance = 13.0f;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererPreLit", 0, WorldRendererPreLit, IWorldRenderer)

render::handle_t WorldRendererPreLit::ms_techniqueDefault = 0;
render::handle_t WorldRendererPreLit::ms_techniqueLight = 0;
render::handle_t WorldRendererPreLit::ms_techniqueDepth = 0;
render::handle_t WorldRendererPreLit::ms_techniqueShadow = 0;
render::handle_t WorldRendererPreLit::ms_handleProjection = 0;

WorldRendererPreLit::WorldRendererPreLit()
:	m_count(0)
{
	// Techniques
	ms_techniqueDefault = render::getParameterHandle(L"Default");
	ms_techniqueLight = render::getParameterHandle(L"Light");
	ms_techniqueDepth = render::getParameterHandle(L"Depth");
	ms_techniqueShadow = render::getParameterHandle(L"Shadow");

	// Global parameters.
	ms_handleProjection = render::getParameterHandle(L"Projection");
}

bool WorldRendererPreLit::create(
	const WorldRenderSettings& settings,
	WorldEntityRenderers* entityRenderers,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::IRenderView* renderView,
	uint32_t multiSample,
	uint32_t frameCount
)
{
	T_ASSERT_M (renderView, L"Render view required");

	m_settings = settings;
	m_renderView = renderView;
	m_frames.resize(frameCount);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create "depth map" target.
	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR8G8B8A8;

		m_depthTargetSet = renderSystem->createRenderTargetSet(desc);

		if (!m_depthTargetSet && multiSample > 0)
		{
			desc.multiSample = 0;
			desc.createDepthStencil = true;
			desc.usingPrimaryDepthStencil = false;

			m_depthTargetSet = renderSystem->createRenderTargetSet(desc);
			if (m_depthTargetSet)
				log::warning << L"MSAA depth render target unsupported; may cause poor performance" << Endl;
		}

		if (!m_depthTargetSet)
		{
			log::warning << L"Unable to create depth render target; depth disabled" << Endl;
			m_settings.depthPassEnabled = false;
			m_settings.shadowsEnabled = false;
		}
	}

	// Create "shadow map" targets.
	if (m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		// Create shadow map target.
		desc.count = 1;
		desc.width =
		desc.height = m_settings.shadowMapResolution;
		desc.multiSample = 0;
		desc.createDepthStencil = true;
		desc.usingPrimaryDepthStencil = false;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR8G8B8A8;

		switch (m_settings.shadowsQuality)
		{
		case WorldRenderSettings::SqLow:
			desc.width /= 4;
			desc.height /= 4;
			break;

		case WorldRenderSettings::SqMedium:
			desc.width /= 2;
			desc.height /= 2;
			break;
		}

		m_shadowTargetSet = renderSystem->createRenderTargetSet(desc);

		// Determine shadow mask size; high quality is same as entire screen.
		if (
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
		m_shadowMaskTargetSet = renderSystem->createRenderTargetSet(desc);

		if (m_shadowTargetSet && m_shadowMaskTargetSet)
		{
			resource::Proxy< PostProcessSettings > shadowMaskProjectionSettings;

			switch (m_settings.shadowsQuality)
			{
			case WorldRenderSettings::SqNoFilter:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsNoFilter;
				break;
			case WorldRenderSettings::SqLow:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsLow;
				break;
			case WorldRenderSettings::SqMedium:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsMedium;
				break;
			case WorldRenderSettings::SqHigh:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsHigh;
				break;
			case WorldRenderSettings::SqHighest:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsHighest;
				break;
			}

			resourceManager->bind(shadowMaskProjectionSettings);

			m_shadowMaskProjection = new PostProcess();
			if (!m_shadowMaskProjection->create(
				shadowMaskProjectionSettings,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height
			))
			{
				log::warning << L"Unable to create shadow projection process; shadows disabled" << Endl;
				m_settings.shadowsEnabled = false;
			}
		}
		else
		{
			log::warning << L"Unable to create shadow render targets; shadows disabled" << Endl;
			m_settings.shadowsEnabled = false;
		}

		// Ensure targets are destroyed if something went wrong in setup.
		if (!m_settings.shadowsEnabled)
		{
			safeDestroy(m_shadowTargetSet);
			safeDestroy(m_shadowMaskTargetSet);
		}
	}

	// Create light map target.
	render::RenderTargetSetCreateDesc desc;

	desc.count = 1;
	desc.width = width;
	desc.height = height;
	desc.multiSample = multiSample;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = true;
	desc.preferTiled = true;
	desc.targets[0].format = render::TfR8G8B8A8;

	m_lightTargetSet = renderSystem->createRenderTargetSet(desc);
	if (!m_lightTargetSet)
	{
		log::error << L"Unable to create light render target" << Endl;
		return false;
	}

	// Allocate "depth" context.
	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->depth = new WorldContext(entityRenderers);
	}

	// For each possible light.
	for (uint32_t i = 0; i < WorldRenderView::MaxLightCount; ++i)
	{
		// Allocate "shadow" contexts.
		if (m_settings.shadowsEnabled)
		{
			for (AlignedVector< Frame >::iterator j = m_frames.begin(); j != m_frames.end(); ++j)
				j->shadow[i] = new WorldContext(entityRenderers);
		}

		// Allocate "light" contexts.
		for (AlignedVector< Frame >::iterator j = m_frames.begin(); j != m_frames.end(); ++j)
			j->light[i] = new WorldContext(entityRenderers);
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(entityRenderers);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	m_count = 0;
	return true;
}

void WorldRendererPreLit::destroy()
{
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		for (uint32_t j = 0; j < WorldRenderView::MaxLightCount; ++j)
		{
			i->shadow[j] = 0;
			i->light[j] = 0;
		}
		i->visual = 0;
		i->depth = 0;
	}

	safeDestroy(m_shadowMaskProjection);
	safeDestroy(m_shadowMaskTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_depthTargetSet);

	m_renderView = 0;
}

void WorldRendererPreLit::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
{
	float viewNearZ = m_settings.viewNearZ;
	float viewFarZ = m_settings.viewFarZ;
	float depthRange = m_settings.depthRange;

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
	float depthRange = m_settings.depthRange;

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
	if (f.haveDepth)
		f.depth->getRenderContext()->flush();

	for (uint32_t i = 0; i < f.lightCount; ++i)
	{
		if (f.haveShadows[i])
			f.shadow[i]->getRenderContext()->flush();
	}

	for (uint32_t i = 0; i < f.lightCount; ++i)
		f.light[i]->getRenderContext()->flush();

	f.visual->getRenderContext()->flush();

	// Build this frame's render contexts.
	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		WorldRenderView depthRenderView = worldRenderView;
		depthRenderView.resetLights();

		WorldRenderPassPreLit depthPass(
			ms_techniqueDepth,
			depthRenderView,
			m_settings.depthRange,
			0,
			0,
			0
		);
		f.depth->build(depthRenderView, depthPass, entity);
		f.depth->flush(depthRenderView, depthPass);

		f.haveDepth = true;
	}
	else
		f.haveDepth = false;

	if (m_settings.shadowsEnabled)
		buildLightWithShadows(worldRenderView, entity, frame);
	else
		buildLightWithNoShadows(worldRenderView, entity, frame);

	buildVisual(worldRenderView, entity, frame);

	m_count++;
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
	if ((flags & WrfDepthMap) != 0 && f.haveDepth)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Depth");
		if (m_renderView->begin(m_depthTargetSet, 0))
		{
			const float depthColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			m_renderView->clear(render::CfColor | render::CfDepth, depthColor, 1.0f, 0);
			f.depth->getRenderContext()->render(m_renderView, render::RfOpaque, &programParams);
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}
	else if (!f.haveDepth)
	{
		// No depth pass; ensure primary depth is cleared.
		const float nullColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_renderView->clear(render::CfDepth, nullColor, 1.0f, 0);
	}

	// Render light map.
	if (f.lightCount > 0)
	{
		for (uint32_t i = 0; i < f.lightCount; ++i)
		{
			// Render shadow mask.
			if ((flags & WrfShadowMap) != 0 && f.haveShadows[i])
			{
				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
				if (m_renderView->begin(m_shadowTargetSet, 0))
				{
					const float shadowClear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
					m_renderView->clear(render::CfColor | render::CfDepth, shadowClear, 1.0f, 0);
					f.shadow[i]->getRenderContext()->render(m_renderView, render::RfOpaque, 0);
					m_renderView->end();
				}
				T_RENDER_POP_MARKER(m_renderView);

				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask");
				if (m_renderView->begin(m_shadowMaskTargetSet, 0))
				{
					PostProcessStep::Instance::RenderParams params;

					params.viewFrustum = f.viewFrustum;
					params.viewToLight = f.viewToLightSpace[i];
					params.projection = projection;
					params.squareProjection = f.squareProjection[i];
					params.depthRange = m_settings.depthRange;
					params.shadowFarZ = m_settings.shadowFarZ;
					params.shadowMapBias = m_settings.shadowMapBias;
					params.deltaTime = 0.0f;

					m_shadowMaskProjection->render(
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

			// Render light.
			T_RENDER_PUSH_MARKER(m_renderView, "World: Light map");
			if (m_renderView->begin(m_lightTargetSet, 0))
			{
				if (i == 0)
				{
					const float lightClear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					m_renderView->clear(render::CfColor, lightClear, 0.0f, 0);
				}
				f.light[i]->getRenderContext()->render(m_renderView, render::RfOpaque | render::RfAlphaBlend, 0);
				m_renderView->end();
			}
			T_RENDER_POP_MARKER(m_renderView);
		}
	}
	else
	{
		// No active lights; ensure light map is cleared.
		T_RENDER_PUSH_MARKER(m_renderView, "World: Light map");
		if (m_renderView->begin(m_lightTargetSet, 0))
		{
			const float lightClear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_renderView->clear(render::CfColor, lightClear, 0.0f, 0);
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render visuals.
	if ((flags & (WrfVisualOpaque | WrfVisualAlphaBlend)) != 0)
	{
		uint32_t renderFlags = render::RfOverlay;

		if (flags & WrfVisualOpaque)
			renderFlags |= render::RfOpaque;
		if (flags & WrfVisualAlphaBlend)
			renderFlags |= render::RfAlphaBlend;

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual");
		f.visual->getRenderContext()->render(m_renderView, renderFlags, &programParams);
		T_RENDER_POP_MARKER(m_renderView);
	}

	m_globalContext->flush();
}

void WorldRendererPreLit::getTargets(RefArray< render::ITexture >& outTargets) const
{
	outTargets.resize(4);
	outTargets[0] = m_depthTargetSet ? m_depthTargetSet->getColorTexture(0) : 0;
	outTargets[1] = m_shadowTargetSet ? m_shadowTargetSet->getColorTexture(0) : 0;
	outTargets[2] = m_shadowMaskTargetSet ? m_shadowMaskTargetSet->getColorTexture(0) : 0;
	outTargets[3] = m_lightTargetSet ? m_lightTargetSet->getColorTexture(0) : 0;
}

void WorldRendererPreLit::buildLightWithShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();
	Vector4 eyePosition = viewInverse.translation();

	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const WorldRenderView::Light& light = worldRenderView.getLight(i);
		if (light.type == WorldRenderView::LtDirectional)
		{
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;
			Matrix44 shadowLightSquareProjection = Matrix44::identity();
			Frustum shadowFrustum;

			switch (m_settings.shadowsProjection)
			{
			case WorldRenderSettings::SpBox:
				calculateBoxSMProj(
					m_settings,
					viewInverse,
					light.position,
					light.direction,
					viewFrustum,
					shadowBox,
					shadowLightView,
					shadowLightProjection,
					shadowFrustum
				);
				break;

			case WorldRenderSettings::SpLiSP:
				calculateLiSPSMProj(
					m_settings,
					viewInverse,
					light.position,
					light.direction,
					viewFrustum,
					shadowLightView,
					shadowLightProjection,
					shadowFrustum
				);
				break;

			case WorldRenderSettings::SpTrapezoid:
				calculateTSMProj(
					m_settings,
					viewInverse,
					light.position,
					light.direction,
					viewFrustum,
					shadowLightView,
					shadowLightProjection,
					shadowLightSquareProjection,
					shadowFrustum
				);
				break;

			case WorldRenderSettings::SpUniform:
				calculateUniformSMProj(
					m_settings,
					viewInverse,
					light.position,
					light.direction,
					viewFrustum,
					shadowLightView,
					shadowLightProjection,
					shadowFrustum
				);
				break;
			}

			f.viewToLightSpace[i] = shadowLightProjection * shadowLightView * viewInverse;
			f.squareProjection[i] = shadowLightSquareProjection;

			// Render shadow map.
			WorldRenderView shadowRenderView;
			shadowRenderView.resetLights();
			shadowRenderView.setProjection(shadowLightProjection);
			shadowRenderView.setSquareProjection(shadowLightSquareProjection);
			shadowRenderView.setView(shadowLightView);
			shadowRenderView.setEyePosition(eyePosition);
			shadowRenderView.setViewFrustum(shadowFrustum);
			shadowRenderView.setCullFrustum(shadowFrustum);
			shadowRenderView.setTimes(
				worldRenderView.getTime(),
				worldRenderView.getDeltaTime(),
				worldRenderView.getInterval()
			);

			WorldRenderPassPreLit shadowPass(
				ms_techniqueShadow,
				shadowRenderView,
				m_settings.depthRange,
				0,
				0,
				0
			);
			f.shadow[i]->build(shadowRenderView, shadowPass, entity);
			f.shadow[i]->flush(shadowRenderView, shadowPass);

			f.haveShadows[i] = true;
		}
		else
			f.haveShadows[i] = false;

		// Render lighting.
		WorldRenderView lightRenderView = worldRenderView;
		lightRenderView.resetLights();
		lightRenderView.addLight(light);

		WorldRenderPassPreLit lightPass(
			ms_techniqueLight,
			lightRenderView,
			m_settings.depthRange,
			m_depthTargetSet->getColorTexture(0),
			f.haveShadows[i] ? m_shadowMaskTargetSet->getColorTexture(0) : 0,
			0
		);
		f.light[i]->build(lightRenderView, lightPass, entity);
		f.light[i]->flush(lightRenderView, lightPass);
	}

	f.lightCount = worldRenderView.getLightCount();
}

void WorldRendererPreLit::buildLightWithNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();
	Vector4 eyePosition = viewInverse.translation();

	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const WorldRenderView::Light& light = worldRenderView.getLight(i);

		WorldRenderView lightRenderView = worldRenderView;
		lightRenderView.resetLights();
		lightRenderView.addLight(light);

		WorldRenderPassPreLit lightPass(
			ms_techniqueLight,
			lightRenderView,
			m_settings.depthRange,
			f.haveDepth ? m_depthTargetSet->getColorTexture(0) : 0,
			0,
			0
		);
		f.light[i]->build(lightRenderView, lightPass, entity);
		f.light[i]->flush(lightRenderView, lightPass);
	}

	f.lightCount = worldRenderView.getLightCount();
}

void WorldRendererPreLit::buildVisual(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();
	Vector4 eyePosition = viewInverse.translation();

	worldRenderView.resetLights();
	worldRenderView.setEyePosition(eyePosition);

	WorldRenderPassPreLit defaultPass(
		ms_techniqueDefault,
		worldRenderView,
		m_settings.depthRange,
		f.haveDepth ? m_depthTargetSet->getColorTexture(0) : 0,
		0,
		m_lightTargetSet->getColorTexture(0)
	);
	f.visual->build(worldRenderView, defaultPass, entity);
	f.visual->flush(worldRenderView, defaultPass);

	f.projection = worldRenderView.getProjection();
	f.viewFrustum = worldRenderView.getViewFrustum();
}

	}
}
