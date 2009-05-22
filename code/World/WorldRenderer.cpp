#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Entity/EntityRenderer.h"
#include "World/Entity/Entity.h"
#include "World/PostProcess/PostProcess.h"
#include "Render/RenderSystem.h"
#include "Render/RenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/SimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Core/Math/Random.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

/*! \brief Default light-view projection. */
class DefaultLightViewProjection : public LightViewProjection
{
public:
	virtual void calculateLightViewProjection(
		const WorldRenderSettings& settings,
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		Matrix44& outLightView,
		Matrix44& outLightProjectionOccluders,
		Matrix44& outLightProjectionSelfShadow,
		Frustum& outShadowFrustumOccluders,
		Frustum& outShadowFrustumSelfShadow,
		uint32_t& outShadowPasses
	) const
	{
		// Calculate light axises.
		Vector4 lightAxisZ = -lightDirection.normalized();
		Vector4 lightAxisX = cross(Vector4(0.0f, 1.0f, 0.0f, 0.0f), lightAxisZ).normalized();
		Vector4 lightAxisY = cross(lightAxisZ, lightAxisX).normalized();
		T_ASSERT (cross(lightAxisX, lightAxisY).normalized() == lightAxisZ);

		// Calculate bounding box of view frustum in light space.
		Aabb viewFrustumBox;
		for (int i = 0; i < 8; ++i)
		{
			Vector4 worldCorner = viewInverse * viewFrustum.corners[i];
			Vector4 lightCorner(
				dot3(lightAxisX, worldCorner),
				dot3(lightAxisY, worldCorner),
				dot3(lightAxisZ, worldCorner),
				1.0f
			);
			viewFrustumBox.contain(lightCorner);
		}

		// Update light view matrix with bounding box centered.
		Vector4 center = viewFrustumBox.getCenter();
		Vector4 extent = viewFrustumBox.getExtent() * Vector4(2.0f, 2.0f, 1.0f, 0.0f);

		// Calculate world center of view frustum's bounding box.
		Vector4 worldCenter =
			lightAxisX * center.x() +
			lightAxisY * center.y() +
			lightAxisZ * center.z() +
			Vector4::origo();

		float lightDistance = settings.viewFarZ;

		outLightView = Matrix44(
			lightAxisX,
			lightAxisY,
			lightAxisZ,
			worldCenter - lightAxisZ * Scalar(lightDistance)
		);

		outLightView = outLightView.inverseOrtho();

		outLightProjectionOccluders = orthoLh(
			extent.x(),
			extent.y(),
			0.0f,
			lightDistance - extent.z()
		);

		outLightProjectionSelfShadow = orthoLh(
			extent.x(),
			extent.y(),
			lightDistance - extent.z(),
			lightDistance + extent.z()
		);

		outShadowFrustumOccluders.buildOrtho(
			extent.x(),
			extent.y(),
			0.0f,
			lightDistance - extent.z()
		);

		outShadowFrustumSelfShadow.buildOrtho(
			extent.x(),
			extent.y(),
			lightDistance - extent.z(),
			lightDistance + extent.z()
		);

		// Both passes are valid.
		outShadowPasses = SpOccluders | SpSelf;
	}
};

/*! \brief Random rotation texture.
 *
 * This texture is used to rotate the Poisson distribution
 * disc for each fragment in shadow mapping.
 */
render::SimpleTexture* createRandomRotationTexture(render::RenderSystem* renderSystem)
{
	static Random random;

	uint8_t data[128 * 128 * 4];
	for (uint32_t y = 0; y < 128; ++y)
	{
		for (uint32_t x = 0; x < 128; ++x)
		{
			float angle = (random.nextFloat() * 2.0f - 1.0f) * PI;
			float c = cosf(angle) * 127.5f + 127.5f;
			float s = sinf(angle) * 127.5f + 127.5f;
			data[(x + y * 128) * 4 + 0] = uint8_t(c);
			data[(x + y * 128) * 4 + 1] = uint8_t(s);
			data[(x + y * 128) * 4 + 2] = uint8_t(c);
			data[(x + y * 128) * 4 + 3] = uint8_t(s);
		}
	}

	render::SimpleTextureCreateDesc desc;
	desc.width = 128;
	desc.height = 128;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data;
	desc.initialData[0].pitch = 128 * 4;
	desc.initialData[0].slicePitch = 0;

	return renderSystem->createSimpleTexture(desc);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderer", WorldRenderer, Object)

render::handle_t WorldRenderer::ms_techniqueDefault = 0;
render::handle_t WorldRenderer::ms_techniqueDepth = 0;
render::handle_t WorldRenderer::ms_techniqueShadowMapOccluders = 0;
render::handle_t WorldRenderer::ms_techniqueShadowMapSelfShadow = 0;

WorldRenderer::WorldRenderer()
:	m_lightViewProjection(gc_new< DefaultLightViewProjection >())
,	m_haveDepth(false)
,	m_haveShadows(false)
,	m_time(0.0f)
,	m_count(0)
{
	ms_techniqueDefault = render::getParameterHandle(L"Default");
	ms_techniqueDepth = render::getParameterHandle(L"Depth");
	ms_techniqueShadowMapOccluders = render::getParameterHandle(L"ShadowMapOccluders");
	ms_techniqueShadowMapSelfShadow = render::getParameterHandle(L"ShadowMapSelfShadow");
}

bool WorldRenderer::create(
	const WorldRenderSettings& settings,
	WorldEntityRenderers* entityRenderers,
	render::RenderSystem* renderSystem,
	render::RenderView* renderView,
	const WorldViewPort& worldViewPort,
	int multiSample,
	int frameCount
)
{
	T_ASSERT_M (renderView, L"Render view required");

	m_settings = settings;
	m_renderView = renderView;
	m_worldViewPort = worldViewPort;
	m_frames.resize(frameCount);

	// Create "depth map" target.
	render::RenderTargetSetCreateDesc desc;
	desc.count = 1;
	desc.width = worldViewPort.width;
	desc.height = worldViewPort.height;
	desc.multiSample = 0;
	desc.depthStencil = false;
	desc.targets[0].format = render::TfR16F;

	m_depthTargetSet = renderSystem->createRenderTargetSet(desc);
	if (!m_depthTargetSet)
		return false;

	// Allocate "shadow map" targets.
	uint32_t sliceCount = m_settings.shadowCascadingSlices;
	if (m_settings.shadowFarZ < m_settings.viewFarZ)
		++sliceCount;

	if (m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = sliceCount;
		desc.width =
		desc.height = m_settings.shadowMapResolution;
		desc.multiSample = 0;
		desc.depthStencil = true;
		for (uint32_t i = 0; i < sliceCount; ++i)
			desc.targets[i].format = render::TfR16F;

		m_shadowTargetSet = renderSystem->createRenderTargetSet(desc);
		if (m_shadowTargetSet)
		{
			for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			{
				i->occluders.resize(sliceCount);
				i->selfShadow.resize(sliceCount);
				i->visual.resize(sliceCount);

				for (uint32_t j = 0; j < sliceCount; ++j)
				{
					i->occluders[j] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
					i->selfShadow[j] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
					i->visual[j] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
				}
			}
		}
		else
		{
			log::warning << L"Unable to create shadow render target; shadows disabled" << Endl;
			m_settings.shadowsEnabled = false;
		}
	}

	// Allocate "depth" context.
	if (m_settings.depthPassEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->depth = gc_new< WorldContext >(this, entityRenderers, m_renderView);
	}

	// Allocate "visual" and s"hadow" contexts for each slice.
	if (m_settings.shadowsEnabled)
	{
		// Create render contexts; one for each frame.
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		{
			i->occluders.resize(sliceCount);
			i->selfShadow.resize(sliceCount);
			i->visual.resize(sliceCount);

			for (uint32_t j = 0; j < sliceCount; ++j)
			{
				i->occluders[j] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
				i->selfShadow[j] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
				i->visual[j] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
			}
		}

		// Calculate CSM splits.
		m_splitPositions.resize(m_settings.shadowCascadingSlices + 1);
		for (int i = 1; i < m_settings.shadowCascadingSlices; ++i)
		{
			float idm = float(i) / m_settings.shadowCascadingSlices;
			float log = m_settings.viewNearZ * powf(m_settings.shadowFarZ / m_settings.viewNearZ, idm);
			float uni = m_settings.viewNearZ + (m_settings.shadowFarZ - m_settings.viewNearZ) * idm;
			m_splitPositions[i] = log * m_settings.shadowCascadingLambda + uni * (1.0f - m_settings.shadowCascadingLambda);
		}

		m_splitPositions[0] = m_settings.viewNearZ;
		m_splitPositions[m_settings.shadowCascadingSlices] = m_settings.shadowFarZ;

		// Generate screen-space random rotation texture.
		m_shadowDiscRotation[0] = createRandomRotationTexture(renderSystem);
		m_shadowDiscRotation[1] = createRandomRotationTexture(renderSystem);
	}
	else
	{
		// Shadows not enabled; allocate a single "visual" context.
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		{
			i->visual.resize(1);
			i->visual[0] = gc_new< WorldContext >(this, entityRenderers, m_renderView);
		}
	}

	m_time = 0.0f;
	m_count = 0;

	return true;
}

void WorldRenderer::destroy()
{
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		i->occluders.resize(0);
		i->selfShadow.resize(0);
		i->visual.resize(0);
		i->depth = 0;
	}

	for (int i = 0; i < sizeof_array(m_shadowDiscRotation); ++i)
	{
		if (m_shadowDiscRotation[i])
		{
			m_shadowDiscRotation[i]->destroy();
			m_shadowDiscRotation[i] = 0;
		}
	}

	if (m_shadowTargetSet)
	{
		m_shadowTargetSet->destroy();
		m_shadowTargetSet = 0;
	}
}

void WorldRenderer::createRenderView(WorldRenderView& outRenderView) const
{
	float viewNearZ = m_settings.viewNearZ;
	float viewFarZ = m_settings.viewFarZ;

	Frustum viewFrustum;
	viewFrustum.buildPerspective(m_worldViewPort.fov, m_worldViewPort.aspect, viewNearZ, viewFarZ);

	outRenderView.setViewSize(Vector2(float(m_worldViewPort.width), float(m_worldViewPort.height)));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(perspectiveLh(m_worldViewPort.fov, m_worldViewPort.aspect, viewNearZ, viewFarZ));
}

void WorldRenderer::build(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	worldRenderView.setTime(m_time);

	if (m_settings.depthPassEnabled)
	{
		Frame& f = m_frames[frame];

		WorldRenderView depthRenderView = worldRenderView;
		depthRenderView.setTechnique(ms_techniqueDepth);

		f.depth->build(&depthRenderView, entity);
		f.depth->flush(&depthRenderView);

		m_haveDepth = true;
	}
	else
		m_haveDepth = false;

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

	if ((flags & WrfDepthMap) != 0 && m_haveDepth)
	{
		if (!m_renderView->begin(m_depthTargetSet, 0, true))
			return;

		const float depthColor[] = { m_settings.viewFarZ, m_settings.viewFarZ, m_settings.viewFarZ, m_settings.viewFarZ };
		m_renderView->clear(render::CfColor | render::CfDepth, depthColor, 1.0f, 0);
		f.depth->getRenderContext()->render(render::RenderContext::RfOpaque);
		m_renderView->end();
	}

	if ((flags & WrfShadowMap) != 0 && m_haveShadows)
	{
		float sliceDenom = float(m_settings.shadowCascadingSlices);
		if (m_settings.shadowFarZ < m_settings.viewFarZ)
			++sliceDenom;

		const float shadowClear[] = { m_settings.shadowFarZ, m_settings.shadowFarZ, m_settings.shadowFarZ, m_settings.shadowFarZ };
		for (int slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
		{
			if (!m_renderView->begin(m_shadowTargetSet, slice, false))
				continue;

			m_renderView->clear(render::CfColor | render::CfDepth, shadowClear, 1.0f, 0);
			f.selfShadow[slice]->getRenderContext()->render(render::RenderContext::RfOpaque);
			f.occluders[slice]->getRenderContext()->render(render::RenderContext::RfOpaque);
			m_renderView->end();
		}
	}

	if (m_haveShadows)
	{
		render::Viewport viewport = m_renderView->getViewport();
		render::Viewport sliceViewport = viewport;

		float sliceDenom = float(m_settings.shadowCascadingSlices);
		if (m_settings.shadowFarZ < m_settings.viewFarZ)
			++sliceDenom;

		// Render opaque visuals.
		if (flags & WrfVisualOpaque)
		{
			for (int slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
			{
				sliceViewport.nearZ = viewport.nearZ + float(slice * (viewport.farZ - viewport.nearZ)) / sliceDenom;
				sliceViewport.farZ = viewport.nearZ + float((slice + 1) * (viewport.farZ - viewport.nearZ)) / sliceDenom;

				m_renderView->setViewport(sliceViewport);
				f.visual[slice]->getRenderContext()->render(render::RenderContext::RfOpaque);
			}
		}

		// Render non shadow slice.
		if (m_settings.shadowFarZ < m_settings.viewFarZ)
		{
			sliceViewport.nearZ = viewport.nearZ + float(m_settings.shadowCascadingSlices * (viewport.farZ - viewport.nearZ)) / sliceDenom;
			sliceViewport.farZ = viewport.nearZ + float((m_settings.shadowCascadingSlices + 1) * (viewport.farZ - viewport.nearZ)) / sliceDenom;

			m_renderView->setViewport(sliceViewport);
			f.visual[m_settings.shadowCascadingSlices]->getRenderContext()->render(render::RenderContext::RfOpaque | render::RenderContext::RfAlphaBlend);
		}

		// Render alpha blend visuals.
		if (flags & WrfVisualAlphaBlend)
		{
			for (int slice = m_settings.shadowCascadingSlices - 1; slice >= 0; --slice)
			{
				sliceViewport.nearZ = viewport.nearZ + float(slice * (viewport.farZ - viewport.nearZ)) / sliceDenom;
				sliceViewport.farZ = viewport.nearZ + float((slice + 1) * (viewport.farZ - viewport.nearZ)) / sliceDenom;

				m_renderView->setViewport(sliceViewport);
				f.visual[slice]->getRenderContext()->render(render::RenderContext::RfAlphaBlend);
			}
		}

		m_renderView->setViewport(viewport);
	}
	else
	{
		uint32_t renderFlags = 0;

		if (flags & WrfVisualOpaque)
			renderFlags |= render::RenderContext::RfOpaque;
		if (flags & WrfVisualAlphaBlend)
			renderFlags |= render::RenderContext::RfAlphaBlend;

		f.visual[0]->getRenderContext()->render(renderFlags);
	}
}

void WorldRenderer::flush(int frame)
{
	Frame& f = m_frames[frame];

	if (m_haveDepth)
		f.depth->getRenderContext()->flush();

	if (m_haveShadows)
	{
		for (int slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
		{
			f.occluders[slice]->getRenderContext()->flush();
			f.selfShadow[slice]->getRenderContext()->flush();
		}

		for (int slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
			f.visual[slice]->getRenderContext()->flush();

		if (m_settings.shadowFarZ < m_settings.viewFarZ)
			f.visual[m_settings.shadowCascadingSlices]->getRenderContext()->flush();
	}
	else
	{
		f.visual[0]->getRenderContext()->flush();
	}
}

void WorldRenderer::buildShadows(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 projection = worldRenderView.getProjection();
	Matrix44 viewInverse = worldRenderView.getView().inverseOrtho();
	Vector4 lightPosition = worldRenderView.getLightPosition(0);
	Vector4 lightDirection = worldRenderView.getLightDirection(0);
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Frustum cullFrustum = worldRenderView.getCullFrustum();

	const Vector4& eyePosition = viewInverse.translation();

	Matrix44 shadowLightView;
	Matrix44 shadowLightProjectionOccluders, shadowLightProjectionSelfShadow;
	Frustum shadowFrustumOccluders, shadowFrustumSelfShadow;

	WorldRenderView shadowRenderView;

	for (int slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
	{
		float zn = max(m_splitPositions[slice], m_settings.viewNearZ);
		float zf = min(m_splitPositions[slice + 1], m_settings.shadowFarZ);

		Frustum sliceFrustum = viewFrustum;
		sliceFrustum.setNearZ(Scalar(zn));
		sliceFrustum.setFarZ(Scalar(zf));

		uint32_t shadowPasses = 0;

		// Calculate light-view projection matrices and frustums.
		m_lightViewProjection->calculateLightViewProjection(
			m_settings,
			viewInverse,
			lightPosition,
			lightDirection,
			sliceFrustum,
			shadowLightView,
			shadowLightProjectionOccluders,
			shadowLightProjectionSelfShadow,
			shadowFrustumOccluders,
			shadowFrustumSelfShadow,
			shadowPasses
		);

		if (shadowPasses & LightViewProjection::SpOccluders)
		{
			// Shadow map occluders pass.
			shadowRenderView.resetLights();
			shadowRenderView.setTechnique(ms_techniqueShadowMapOccluders);
			shadowRenderView.setProjection(shadowLightProjectionOccluders);
			shadowRenderView.setView(shadowLightView);
			shadowRenderView.setEyePosition(eyePosition);
			shadowRenderView.setViewFrustum(shadowFrustumOccluders);
			shadowRenderView.setCullFrustum(shadowFrustumOccluders);
			shadowRenderView.setTime(m_time);

			f.occluders[slice]->build(&shadowRenderView, entity);
			f.occluders[slice]->flush(&shadowRenderView);
		}

		if (shadowPasses & LightViewProjection::SpSelf)
		{
			// Shadow map self shadowing pass.
			shadowRenderView.resetLights();
			shadowRenderView.setTechnique(ms_techniqueShadowMapSelfShadow);
			shadowRenderView.setProjection(shadowLightProjectionSelfShadow);
			shadowRenderView.setView(shadowLightView);
			shadowRenderView.setEyePosition(eyePosition);
			shadowRenderView.setViewFrustum(shadowFrustumSelfShadow);
			shadowRenderView.setCullFrustum(shadowFrustumSelfShadow);
			shadowRenderView.setTime(m_time);

			f.selfShadow[slice]->build(&shadowRenderView, entity);
			f.selfShadow[slice]->flush(&shadowRenderView);
		}

		// Modify perspective transform so near and far clip planes are accurate.
		Matrix44 sliceProjection = projection;
		sliceProjection.e33 = zf / (zf - zn);
		sliceProjection.e43 = -zn * zf / (zf - zn);

		// Render visuals.
		worldRenderView.resetLights();
		worldRenderView.setProjection(sliceProjection);
		worldRenderView.setCullFrustum(sliceFrustum);
		worldRenderView.setViewToLightSpace(viewInverse * shadowLightView * shadowLightProjectionSelfShadow);
		worldRenderView.setEyePosition(eyePosition);
		worldRenderView.setShadowMap(
			m_shadowTargetSet->getColorTexture(slice),
			m_settings.shadowMapBias,
			slice
		);
		worldRenderView.setShadowMapDiscRotation(m_shadowDiscRotation[m_count & 1]);
		if (m_haveDepth)
			worldRenderView.setDepthMap(m_depthTargetSet->getColorTexture(0));
		else
			worldRenderView.setDepthMap(0);

		f.visual[slice]->build(&worldRenderView, entity);
		f.visual[slice]->flush(&worldRenderView);
	}

	// Render non-shadow slice.
	if (m_settings.shadowFarZ < m_settings.viewFarZ)
	{
		float zn = m_settings.shadowFarZ;
		float zf = m_settings.viewFarZ;

		Frustum sliceFrustum = viewFrustum;
		sliceFrustum.setNearZ(Scalar(zn));
		sliceFrustum.setFarZ(Scalar(zf));

		// Modify perspective transform so near and far clip planes are accurate.
		Matrix44 sliceProjection = projection;
		sliceProjection.e33 = zf / (zf - zn);
		sliceProjection.e43 = -zn * zf / (zf - zn);

		worldRenderView.resetLights();
		worldRenderView.setProjection(sliceProjection);
		worldRenderView.setCullFrustum(sliceFrustum);
		worldRenderView.setEyePosition(eyePosition);
		worldRenderView.setViewToLightSpace(Matrix44::identity());
		worldRenderView.setShadowMap(0, 0.0f, 0);
		if (m_haveDepth)
			worldRenderView.setDepthMap(m_depthTargetSet->getColorTexture(0));
		else
			worldRenderView.setDepthMap(0);

		f.visual[m_settings.shadowCascadingSlices]->build(&worldRenderView, entity);
		f.visual[m_settings.shadowCascadingSlices]->flush(&worldRenderView);
	}

	// Reset view's original state.
	worldRenderView.setProjection(projection);
	worldRenderView.setCullFrustum(cullFrustum);
	worldRenderView.setViewFrustum(viewFrustum);

	m_haveShadows = true;
}

void WorldRenderer::buildNoShadows(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverseOrtho();
	const Vector4& eyePosition = viewInverse.translation();

	worldRenderView.resetLights();
	worldRenderView.setEyePosition(eyePosition);
	worldRenderView.setViewToLightSpace(Matrix44::identity());
	worldRenderView.setShadowMap(0, 0.0f, 0);
	if (m_haveDepth)
		worldRenderView.setDepthMap(m_depthTargetSet->getColorTexture(0));
	else
		worldRenderView.setDepthMap(0);

	f.visual[0]->build(&worldRenderView, entity);
	f.visual[0]->flush(&worldRenderView);

	m_haveShadows = false;
}

	}
}
