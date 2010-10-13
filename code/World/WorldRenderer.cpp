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
#include "World/BoxSMProj.h"
#include "World/LiSPSMProj.h"
#include "World/TSMProj.h"
#include "World/UniformSMProj.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/Entity.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcess.h"

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderer", WorldRenderer, Object)

render::handle_t WorldRenderer::ms_techniqueDefault = 0;
render::handle_t WorldRenderer::ms_techniqueDepth = 0;
render::handle_t WorldRenderer::ms_techniqueShadow = 0;
render::handle_t WorldRenderer::ms_handleProjection = 0;

WorldRenderer::WorldRenderer()
:	m_count(0)
,	m_depthTargetHaveOwnZBuffer(false)
{
	ms_techniqueDefault = render::getParameterHandle(L"Default");
	ms_techniqueDepth = render::getParameterHandle(L"Depth");
	ms_techniqueShadow = render::getParameterHandle(L"Shadow");
	ms_handleProjection = render::getParameterHandle(L"Projection");
}

bool WorldRenderer::create(
	const WorldRenderSettings* settings,
	WorldEntityRenderers* entityRenderers,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::IRenderView* renderView,
	int multiSample,
	int frameCount
)
{
	T_ASSERT_M (settings, L"No world renderer settings");
	T_ASSERT_M (renderView, L"Render view required");

	m_settings = *settings;
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
		desc.depthStencil = false;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR8G8B8A8;

		m_depthTargetSet = renderSystem->createRenderTargetSet(desc);

		if (!m_depthTargetSet && multiSample > 0)
		{
			desc.multiSample = 0;
			desc.depthStencil = true;

			m_depthTargetSet = renderSystem->createRenderTargetSet(desc);
			if (m_depthTargetSet)
			{
				log::warning << L"MSAA depth render target unsupported; may cause poor performance" << Endl;
				m_depthTargetHaveOwnZBuffer = true;
			}
		}

		if (!m_depthTargetSet)
		{
			log::warning << L"Unable to create depth render target; depth disabled" << Endl;
			m_settings.depthPassEnabled = false;
		}
	}

	// Allocate "shadow map" targets.
	if (m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		// Create shadow map target.
		desc.count = 1;
		desc.width =
		desc.height = m_settings.shadowMapResolution;
		desc.multiSample = 0;
		desc.depthStencil = true;
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
		desc.depthStencil = false;
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

	// Allocate "depth" context.
	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->depth = new WorldContext(this, entityRenderers);
	}

	// Allocate "shadow" contexts for each slice.
	if (m_settings.shadowsEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->shadow = new WorldContext(this, entityRenderers);
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(this, entityRenderers);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	m_count = 0;
	return true;
}

void WorldRenderer::destroy()
{
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		i->shadow = 0;
		i->visual = 0;
		i->depth = 0;
	}

	for (int i = 0; i < sizeof_array(m_shadowDiscRotation); ++i)
		safeDestroy(m_shadowDiscRotation[i]);

	safeDestroy(m_shadowMaskProjection);
	safeDestroy(m_shadowMaskTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_depthTargetSet);

	m_renderView = 0;
}

void WorldRenderer::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
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
	outRenderView.setDepthRange(depthRange);
}

void WorldRenderer::createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const
{
	float viewFarZ = m_settings.viewFarZ;
	float depthRange = m_settings.depthRange;

	Frustum viewFrustum;
	viewFrustum.buildOrtho(worldView.width, worldView.height, -viewFarZ, viewFarZ);

	outRenderView.setViewSize(Vector2(worldView.width, worldView.height));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(orthoLh(worldView.width, worldView.height, -viewFarZ, viewFarZ));
	outRenderView.setDepthRange(depthRange);
}

void WorldRenderer::build(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	if (f.haveDepth)
		f.depth->getRenderContext()->flush();

	if (f.haveShadows)
		f.shadow->getRenderContext()->flush();

	f.visual->getRenderContext()->flush();

	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		f.depthRenderView = worldRenderView;
		f.depthRenderView.setTechnique(ms_techniqueDepth);

		f.depth->build(&f.depthRenderView, entity);
		f.depth->flush(&f.depthRenderView);

		f.haveDepth = true;
	}
	else
		f.haveDepth = false;

	if (m_settings.shadowsEnabled)
		buildShadows(worldRenderView, entity, frame);
	else
		buildNoShadows(worldRenderView, entity, frame);

	m_count++;
}

void WorldRenderer::render(uint32_t flags, int frame, render::EyeType eye)
{
	Frame& f = m_frames[frame];
	Matrix44 projection;

	// Prepare stereoscopic projection.
	if (eye != render::EtCyclop)
	{
		const float c_interocularDistance = 10.0f;
		const float c_distortionValue = 0.8f;
		const float c_screenPlaneDistance = 12.0f;

		float screenWidth = m_renderView->getWidth();
		
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

	// Render shadow map.
	if (eye == render::EtCyclop || eye == render::EtLeft)
	{
		if ((flags & WrfShadowMap) != 0 && f.haveShadows)
		{
			T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
			if (m_renderView->begin(m_shadowTargetSet, 0, false))
			{
				const float shadowClear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				m_renderView->clear(render::CfColor | render::CfDepth, shadowClear, 1.0f, 0);
				f.shadow->getRenderContext()->render(m_renderView, render::RfOpaque, 0);
				m_renderView->end();
			}
			T_RENDER_POP_MARKER(m_renderView);
		}
	}

	// Render depth map; use as z-prepass if able to share depth buffer with primary.
	if ((flags & WrfDepthMap) != 0 && f.haveDepth)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Depth");
		if (m_renderView->begin(m_depthTargetSet, 0, !m_depthTargetHaveOwnZBuffer))
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

	// Render shadow mask.
	if ((flags & WrfShadowMap) != 0 && f.haveShadows)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask");
		if (m_renderView->begin(m_shadowMaskTargetSet, 0, false))
		{
			PostProcessStep::Instance::RenderParams params;

			params.viewFrustum = f.viewFrustum;
			params.viewToLight = f.viewToLightSpace;
			params.projection = projection;
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

void WorldRenderer::buildShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	const WorldRenderView::Light& light = worldRenderView.getLight(0);
	if (light.type != WorldRenderView::LtDirectional)
	{
		// Only primary light as directional enables shadows; do no-shadows path instead.
		buildNoShadows(worldRenderView, entity, frame);
		return;
	}

	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb shadowBox = worldRenderView.getShadowBox();
	
	Vector4 eyePosition = viewInverse.translation();

	Matrix44 shadowLightView;
	Matrix44 shadowLightProjection;
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

	f.viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;

	// Render shadow map.
	f.shadowRenderView.resetLights();
	f.shadowRenderView.setTechnique(ms_techniqueShadow);
	f.shadowRenderView.setProjection(shadowLightProjection);
	f.shadowRenderView.setView(shadowLightView);
	f.shadowRenderView.setEyePosition(eyePosition);
	f.shadowRenderView.setViewFrustum(shadowFrustum);
	f.shadowRenderView.setCullFrustum(shadowFrustum);
	f.shadowRenderView.setDepthRange(m_settings.depthRange);
	f.shadowRenderView.setTimes(
		worldRenderView.getTime(),
		worldRenderView.getDeltaTime(),
		worldRenderView.getInterval()
	);

	f.shadow->build(&f.shadowRenderView, entity);
	f.shadow->flush(&f.shadowRenderView);

	// Render visuals.
	worldRenderView.resetLights();
	worldRenderView.setEyePosition(eyePosition);
	worldRenderView.setShadowMask(m_shadowMaskTargetSet->getColorTexture(0));
	if (f.haveDepth)
		worldRenderView.setDepthMap(m_depthTargetSet->getColorTexture(0));
	else
		worldRenderView.setDepthMap(0);

	f.visual->build(&worldRenderView, entity);
	f.visual->flush(&worldRenderView);

	f.projection = worldRenderView.getProjection();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.haveShadows = true;
}

void WorldRenderer::buildNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverseOrtho();
	Vector4 eyePosition = viewInverse.translation();

	worldRenderView.resetLights();
	worldRenderView.setEyePosition(eyePosition);
	worldRenderView.setShadowMask(0);
	if (f.haveDepth)
		worldRenderView.setDepthMap(m_depthTargetSet->getColorTexture(0));
	else
		worldRenderView.setDepthMap(0);

	f.visual->build(&worldRenderView, entity);
	f.visual->flush(&worldRenderView);

	f.projection = worldRenderView.getProjection();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.haveShadows = false;
}

	}
}
