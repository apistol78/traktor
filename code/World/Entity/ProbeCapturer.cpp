#include "Render/ICubeTexture.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Entity/ProbeCapturer.h"

namespace traktor
{
	namespace world
	{
		namespace
		{
		
const int32_t c_faceSize = 128;

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeCapturer", ProbeCapturer, Object)

ProbeCapturer::ProbeCapturer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

bool ProbeCapturer::create()
{
	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 6;
	rtscd.width = c_faceSize;
	rtscd.height = c_faceSize;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = true;
	rtscd.usingDepthStencilAsTexture = false;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.preferTiled = false;
	rtscd.ignoreStencil = true;
	rtscd.generateMips = false;
	rtscd.targets[0].format = render::TfR32G32B32A32F;
	rtscd.targets[0].sRGB = false;
	rtscd.targets[1].format = render::TfR32G32B32A32F;
	rtscd.targets[1].sRGB = false;
	rtscd.targets[2].format = render::TfR32G32B32A32F;
	rtscd.targets[2].sRGB = false;
	rtscd.targets[3].format = render::TfR32G32B32A32F;
	rtscd.targets[3].sRGB = false;
	rtscd.targets[4].format = render::TfR32G32B32A32F;
	rtscd.targets[4].sRGB = false;
	rtscd.targets[5].format = render::TfR32G32B32A32F;
	rtscd.targets[5].sRGB = false;

	m_renderTargetSet = m_renderSystem->createRenderTargetSet(rtscd);
	if (!m_renderTargetSet)
		return false;

	m_data.resize(c_faceSize * c_faceSize * 4 * sizeof(float));
	return true;
}

void ProbeCapturer::build(
	WorldEntityRenderers* entityRenderers,
	Entity* rootEntity,
	const Vector4& pivot
)
{
	// Lazy create world renderer, need to access entity renderers.
	if (!m_worldRenderer)
	{
		m_worldRenderer = new WorldRendererDeferred();

		world::WorldRenderSettings wrs;
		wrs.viewNearZ = 0.01f;
		wrs.viewFarZ = 100.0f;
		wrs.linearLighting = true;
		wrs.exposureBias = 1.0f;
		wrs.fog = false;

		world::WorldCreateDesc wcd;
		wcd.worldRenderSettings = &wrs;
		wcd.entityRenderers = entityRenderers;
		wcd.toneMapQuality = world::QuDisabled;
		wcd.motionBlurQuality = world::QuDisabled;
		wcd.reflectionsQuality = world::QuDisabled;
		wcd.shadowsQuality = world::QuDisabled;
		wcd.ambientOcclusionQuality = world::QuDisabled;
		wcd.antiAliasQuality = world::QuDisabled;
		wcd.imageProcessQuality = world::QuDisabled;
		wcd.width = c_faceSize;
		wcd.height = c_faceSize;
		wcd.multiSample = 0;
		wcd.frameCount = 6;
		wcd.gamma = 1.0f;
		wcd.allTargetsPersistent = false;
		wcd.sharedDepthStencil = m_renderTargetSet;

		if (!m_worldRenderer->create(
			m_resourceManager,
			m_renderSystem,
			wcd
		))
		{
			m_worldRenderer = nullptr;
			return;
		}
	}

	for (int32_t face = 0; face < 6; ++face)
	{
		Matrix44 view;
		switch (face)
		{
		case 0:	// +X
			view = rotateY(deg2rad(-90.0f));
			break;
		case 1:	// -X
			view = rotateY(deg2rad( 90.0f));
			break;
		case 2:	// +Y
			view = rotateX(deg2rad( 90.0f));
			break;
		case 3: // -Y
			view = rotateX(deg2rad(-90.0f));
			break;
		case 4:	// +Z
			view = Matrix44::identity();
			break;
		case 5:	// -Z
			view = rotateY(deg2rad(180.0f));
			break;
		}

		// Move to pivot point.
		view = view * translate(pivot);

		// Render entities.
		world::WorldRenderView worldRenderView;
		worldRenderView.setPerspective(
			c_faceSize,
			c_faceSize,
			1.0f,
			deg2rad(90.0f),
			0.1f,
			100.0f
		);
		worldRenderView.setTimes(0.0f, 1.0f / 60.0f, 0.0f);
		worldRenderView.setView(view, view);

		m_worldRenderer->attach(rootEntity);
		m_worldRenderer->build(worldRenderView, face);
	}
}

void ProbeCapturer::render(render::IRenderView* renderView)
{
	T_RENDER_PUSH_MARKER(renderView, "Probe Capture");

	render::Clear cl;
	cl.mask = render::CfColor | render::CfDepth;
	cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	cl.depth = 1.0f;

	// Render all faces of cube map.
	for (int32_t face = 0; face < 6; ++face)
	{
		if (renderView->begin(m_renderTargetSet, face, &cl))
		{
			m_worldRenderer->beginRender(renderView, face, cl.colors[0]);
			m_worldRenderer->render(renderView, face);
			m_worldRenderer->endRender(renderView, face, 1.0f / 60.0f);
			renderView->end();
		}
	}

	T_RENDER_POP_MARKER(renderView);
}

void ProbeCapturer::transfer(render::ICubeTexture* probeTexture)
{
	// Download each target.
	for (int32_t side = 0; side < 6; ++side)
	{
		m_renderTargetSet->read(side, m_data.ptr());
		//m_cubeImages[side]->clearAlpha(1.0f);

		render::ITexture::Lock lock;
		if (probeTexture->lock(side, 0, lock))
		{
			//std::memcpy(lock.bits, m_cubeImages[side]->getData(), m_cubeImages[side]->getDataSize());
			std::memcpy(lock.bits, m_data.c_ptr(), m_data.size());
			probeTexture->unlock(side, 0);
		}
	}
}

	}
}