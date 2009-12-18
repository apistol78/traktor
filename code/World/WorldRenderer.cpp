#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/UniformSMProj.h"
#include "World/LiSPSMProj.h"
#include "World/TSMProj.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/Entity.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcess.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Core/Math/Random.h"
#include "Core/Math/Format.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

template < typename T >
void safeDestroy(T& tv)
{
	if (tv)
	{
		tv->destroy();
		tv = 0;
	}
}

const Guid c_shadowMaskProjectionSettingsLow(L"{7D4D38B9-1E43-8046-B1A4-705CFEF9B8EB}");
const Guid c_shadowMaskProjectionSettingsMedium(L"{57FD53AF-547A-9F46-8C94-B4D24EFB63BC}");
const Guid c_shadowMaskProjectionSettingsHigh(L"{FABC4017-4D65-604D-B9AB-9FC03FE3CE43}");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderer", WorldRenderer, Object)

render::handle_t WorldRenderer::ms_techniqueDefault = 0;
render::handle_t WorldRenderer::ms_techniqueDepth = 0;
render::handle_t WorldRenderer::ms_techniqueShadow = 0;
render::handle_t WorldRenderer::ms_techniqueVelocity = 0;

WorldRenderer::WorldRenderer()
:	m_time(0.0f)
,	m_count(0)
{
	ms_techniqueDefault = render::getParameterHandle(L"Default");
	ms_techniqueDepth = render::getParameterHandle(L"Depth");
	ms_techniqueShadow = render::getParameterHandle(L"Shadow");
	ms_techniqueVelocity = render::getParameterHandle(L"Velocity");
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

	render::Viewport viewPort = renderView->getViewport();

	// Create "depth map" target.
	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = viewPort.width;
		desc.height = viewPort.height;
		desc.multiSample = multiSample;
		desc.depthStencil = false;
		desc.targets[0].format = render::TfR16F;

		m_depthTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_depthTargetSet)
		{
			log::warning << L"Unable to create depth render target; depth disabled" << Endl;
			m_settings.depthPassEnabled = false;
		}
	}

	// Create "velocity map" target.
	if (m_settings.velocityPassEnable)
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 2;
		desc.width = viewPort.width;
		desc.height = viewPort.height;
		desc.multiSample = multiSample;
		desc.depthStencil = false;
		desc.targets[0].format = render::TfR16G16B16A16F;
		desc.targets[1].format = render::TfR16G16B16A16F;

		m_velocityTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_velocityTargetSet)
		{
			log::warning << L"Unable to create velocity render target; velocity disabled" << Endl;
			m_settings.velocityPassEnable = false;
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
		desc.targets[0].format = render::TfR16F;
		m_shadowTargetSet = renderSystem->createRenderTargetSet(desc);

		// Create shadow mask target.
		int32_t shadowMaskWidth = viewPort.width / 2;
		int32_t shadowMaskHeight = viewPort.height / 2;

		desc.count = 1;
		desc.width = shadowMaskWidth;
		desc.height = shadowMaskHeight;
		desc.multiSample = 0;
		desc.depthStencil = false;
		desc.targets[0].format = render::TfR8;
		m_shadowMaskTargetSet = renderSystem->createRenderTargetSet(desc);

		if (m_shadowTargetSet && m_shadowMaskTargetSet)
		{
			resource::Proxy< PostProcessSettings > shadowMaskProjectionSettings;

			switch (m_settings.shadowsQuality)
			{
			case WorldRenderSettings::SqLow:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsLow;
				break;
			case WorldRenderSettings::SqMedium:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsMedium;
				break;
			case WorldRenderSettings::SqHigh:
				shadowMaskProjectionSettings = c_shadowMaskProjectionSettingsHigh;
				break;
			}

			resourceManager->bind(shadowMaskProjectionSettings);

			m_shadowMaskProjection = new PostProcess();
			if (!m_shadowMaskProjection->create(
				shadowMaskProjectionSettings,
				resourceManager,
				renderSystem,
				shadowMaskWidth,
				shadowMaskHeight
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
			i->depth = new WorldContext(this, entityRenderers, m_renderView);
	}

	// Allocate "velocity" context.
	if (m_settings.velocityPassEnable)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->velocity = new WorldContext(this, entityRenderers, m_renderView);
	}

	// Allocate "shadow" contexts for each slice.
	if (m_settings.shadowsEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->shadow = new WorldContext(this, entityRenderers, m_renderView);
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(this, entityRenderers, m_renderView);

	m_time = 0.0f;
	m_count = 0;

	return true;
}

void WorldRenderer::destroy()
{
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		i->shadow = 0;
		i->visual = 0;
		i->velocity = 0;
		i->depth = 0;
	}

	for (int i = 0; i < sizeof_array(m_shadowDiscRotation); ++i)
		safeDestroy(m_shadowDiscRotation[i]);

	safeDestroy(m_shadowMaskProjection);
	safeDestroy(m_shadowMaskTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_velocityTargetSet);
	safeDestroy(m_depthTargetSet);

	m_renderView = 0;
}

void WorldRenderer::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
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

void WorldRenderer::createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const
{
	float viewFarZ = m_settings.viewFarZ;

	Frustum viewFrustum;
	viewFrustum.buildOrtho(worldView.width, worldView.height, -viewFarZ, viewFarZ);

	outRenderView.setViewSize(Vector2(worldView.width, worldView.height));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(orthoLh(worldView.width, worldView.height, -viewFarZ, viewFarZ));
}

void WorldRenderer::build(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	worldRenderView.setTime(m_time);

	if (m_settings.depthPassEnabled || m_settings.shadowsEnabled)
	{
		WorldRenderView depthRenderView = worldRenderView;
		depthRenderView.setTechnique(ms_techniqueDepth);

		f.depth->build(&depthRenderView, entity);
		f.depth->flush(&depthRenderView);

		f.haveDepth = true;
	}
	else
		f.haveDepth = false;

	if (m_settings.velocityPassEnable)
	{
		WorldRenderView velocityRenderView = worldRenderView;
		velocityRenderView.setTechnique(ms_techniqueVelocity);

		f.velocity->build(&velocityRenderView, entity);
		f.velocity->flush(&velocityRenderView);

		f.haveVelocity = true;
	}
	else
		f.haveVelocity = false;

	if (m_settings.shadowsEnabled)
		buildShadows(worldRenderView, deltaTime, entity, frame);
	else
		buildNoShadows(worldRenderView, deltaTime, entity, frame);

	f.viewFrustum = worldRenderView.getViewFrustum();
	f.projection = worldRenderView.getProjection();
	f.deltaTime = deltaTime;

	m_time += deltaTime;
	m_count++;
}

void WorldRenderer::render(uint32_t flags, int frame)
{
	Frame& f = m_frames[frame];

	const float nullColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_renderView->clear(render::CfDepth, nullColor, 1.0f, 0);

	if ((flags & WrfDepthMap) != 0 && f.haveDepth)
	{
		if (m_renderView->begin(m_depthTargetSet, 0, true))
		{
			const float depthColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_renderView->clear(render::CfColor, depthColor, 1.0f, 0);
			f.depth->getRenderContext()->render(render::RenderContext::RfOpaque);
			m_renderView->end();
		}
	}

	if ((flags & WrfVelocityMap) != 0 && f.haveVelocity)
	{
		m_velocityTargetSet->swap(0, 1);

		if (m_renderView->begin(m_velocityTargetSet, 0, true))
		{
			const float velocityColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_renderView->clear(render::CfColor, velocityColor, 1.0f, 0);
			f.velocity->getRenderContext()->render(render::RenderContext::RfOpaque);
			m_renderView->end();
		}
	}

	if ((flags & WrfShadowMap) != 0 && f.haveShadows)
	{
		if (m_renderView->begin(m_shadowTargetSet, 0, false))
		{
			const float shadowClear[] = { m_settings.shadowFarZ, m_settings.shadowFarZ, m_settings.shadowFarZ, m_settings.shadowFarZ };
			m_renderView->clear(render::CfColor | render::CfDepth, shadowClear, 1.0f, 0);
			f.shadow->getRenderContext()->render(render::RenderContext::RfOpaque);
			m_renderView->end();
		}

		if (m_renderView->begin(m_shadowMaskTargetSet, 0, false))
		{
			const float maskClear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			m_renderView->clear(render::CfColor, maskClear, 1.0f, 0);
			m_shadowMaskProjection->render(
				m_renderView,
				m_shadowTargetSet,
				m_depthTargetSet,
				0,
				0,
				f.viewFrustum,
				f.viewToLightSpace,
				m_settings.shadowMapBias,
				0.0f
			);
			m_renderView->end();
		}
	}

	if ((flags & (WrfVisualOpaque | WrfVisualAlphaBlend)) != 0)
	{
		uint32_t renderFlags = 0;

		if (flags & WrfVisualOpaque)
			renderFlags |= render::RenderContext::RfOpaque;
		if (flags & WrfVisualAlphaBlend)
			renderFlags |= render::RenderContext::RfAlphaBlend;

		f.visual->getRenderContext()->render(renderFlags);
	}
}

void WorldRenderer::flush(int frame)
{
	Frame& f = m_frames[frame];

	if (f.haveDepth)
		f.depth->getRenderContext()->flush();

	if (f.haveVelocity)
		f.velocity->getRenderContext()->flush();

	if (f.haveShadows)
		f.shadow->getRenderContext()->flush();

	f.visual->getRenderContext()->flush();
}

void WorldRenderer::buildShadows(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 projection = worldRenderView.getProjection();
	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Vector4 lightPosition = worldRenderView.getLightPosition(0);
	Vector4 lightDirection = worldRenderView.getLightDirection(0);
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Frustum cullFrustum = worldRenderView.getCullFrustum();
	
	Vector4 eyePosition = viewInverse.translation();

	Matrix44 shadowLightView;
	Matrix44 shadowLightProjection;
	Frustum shadowFrustum;

	// Adjust view frustum to shadowing far z.
	Frustum shadowViewFrustum = viewFrustum;
	shadowViewFrustum.setFarZ(Scalar(m_settings.shadowFarZ));

	calculateUniformSMProj(
		m_settings,
		viewInverse,
		lightPosition,
		lightDirection,
		shadowViewFrustum,
		shadowLightView,
		shadowLightProjection,
		shadowFrustum
	);

	f.viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;

	// Render shadow map.
	WorldRenderView shadowRenderView;
	shadowRenderView.resetLights();
	shadowRenderView.setTechnique(ms_techniqueShadow);
	shadowRenderView.setProjection(shadowLightProjection);
	shadowRenderView.setView(shadowLightView);
	shadowRenderView.setEyePosition(eyePosition);
	shadowRenderView.setViewFrustum(shadowFrustum);
	shadowRenderView.setCullFrustum(shadowFrustum);
	shadowRenderView.setTime(m_time);

	f.shadow->build(&shadowRenderView, entity);
	f.shadow->flush(&shadowRenderView);

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

	f.haveShadows = true;
}

void WorldRenderer::buildNoShadows(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame)
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

	f.haveShadows = false;
}

	}
}
