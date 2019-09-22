#include "Render/ICubeTexture.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Entity/ProbeCapturer.h"
#include "World/Forward/WorldRendererForward.h"

namespace traktor
{
	namespace world
	{
		namespace
		{
		
const int32_t c_faceSize = 512;

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
	rtscd.count = 1;
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

	m_renderTargetSet = m_renderSystem->createRenderTargetSet(rtscd);
	if (!m_renderTargetSet)
		return false;

	m_data.resize(c_faceSize * c_faceSize * 4 * sizeof(float));
	return true;
}

void ProbeCapturer::build(
	WorldEntityRenderers* entityRenderers,
	Entity* rootEntity,
	const Vector4& pivot,
	int32_t face
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
		wcd.frameCount = 1;
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
	view = view * translate(pivot).inverse();

	// Render entities.
	world::WorldRenderView worldRenderView;
	worldRenderView.setPerspective(
		c_faceSize,
		c_faceSize,
		1.0f,
		deg2rad(90.0f),
		0.01f,
		100.0f
	);
	worldRenderView.setTimes(0.0f, 1.0f / 60.0f, 0.0f);
	worldRenderView.setView(view, view);

	m_worldRenderer->attach(rootEntity);
	m_worldRenderer->build(worldRenderView, 0);
}

void ProbeCapturer::render(render::IRenderView* renderView, int32_t /*face*/)
{
	T_RENDER_PUSH_MARKER(renderView, "Probe Capture");

	render::Clear clear = { 0 };
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;

	if (renderView->begin(m_renderTargetSet, &clear))
	{
		m_worldRenderer->beginRender(renderView, 0, clear.colors[0]);
		m_worldRenderer->render(renderView, 0);
		m_worldRenderer->endRender(renderView, 0, 1.0f / 60.0f);
		renderView->end();
	}

	T_RENDER_POP_MARKER(renderView);
}

void ProbeCapturer::transfer(render::ICubeTexture* probeTexture, int32_t face)
{
	probeTexture->copy(face, 0, m_renderTargetSet->getColorTexture(0));
}

	}
}