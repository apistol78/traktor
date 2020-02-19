#include "Render/ICubeTexture.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "World/IEntityRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Entity/ProbeCapturer.h"
#include "World/Entity/ProbeComponent.h"

namespace traktor
{
	namespace world
	{
		namespace
		{
		
#if !defined(__ANDROID__)
const int32_t c_faceSize = 1024;
#else
const int32_t c_faceSize = 128;
#endif

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeCapturer", ProbeCapturer, Object)

ProbeCapturer::ProbeCapturer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const TypeInfo& worldRendererType)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_worldRendererType(worldRendererType)
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
	rtscd.ignoreStencil = true;
	rtscd.generateMips = false;
	rtscd.targets[0].format = render::TfR11G11B10F;
	rtscd.targets[0].sRGB = false;

	m_renderTargetSet = m_renderSystem->createRenderTargetSet(rtscd, nullptr, T_FILE_LINE_W);
	if (!m_renderTargetSet)
		return false;

	m_data.resize(c_faceSize * c_faceSize * 4 * sizeof(float));
	return true;
}

void ProbeCapturer::setup(
	const WorldEntityRenderers* entityRenderers,
	const Entity* rootEntity,
	const Vector4& pivot,
	int32_t face
)
{
	// Lazy create world renderer, need to access entity renderers.
	if (!m_worldRenderer)
	{
		// Create a clone of world renderer without support to render probes.
		// This prevents nasty cyclic references of entity renderers.
		Ref< WorldEntityRenderers > probeEntityRenderers = new WorldEntityRenderers();
		for (auto er : entityRenderers->get())
		{
			const TypeInfoSet renderableTypes = er->getRenderableTypes();
			if (renderableTypes.find(&type_of< ProbeComponent >()) == renderableTypes.end())
				probeEntityRenderers->add(er);
		}

		m_worldRenderer = mandatory_non_null_type_cast< world::IWorldRenderer* >(m_worldRendererType.createInstance());

		world::WorldRenderSettings wrs;
		wrs.viewNearZ = 0.01f;
		wrs.viewFarZ = 12000.0f;
		wrs.linearLighting = true;
		wrs.exposureMode = world::WorldRenderSettings::EmFixed;
		wrs.exposure = 0.0f;
		wrs.fog = false;

		world::WorldCreateDesc wcd;
		wcd.worldRenderSettings = &wrs;
		wcd.entityRenderers = probeEntityRenderers;
		wcd.toneMapQuality = world::QuMedium;
		wcd.motionBlurQuality = world::QuDisabled;
		wcd.reflectionsQuality = world::QuDisabled;
		wcd.shadowsQuality = world::QuDisabled;
		wcd.ambientOcclusionQuality = world::QuDisabled;
		wcd.antiAliasQuality = world::QuDisabled;
		wcd.imageProcessQuality = world::QuDisabled;
		// wcd.width = c_faceSize;
		// wcd.height = c_faceSize;
		wcd.multiSample = 0;
		wcd.frameCount = 1;
		wcd.gamma = 1.0f;
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
		12000.0f
	);
	worldRenderView.setTimes(0.0f, 1.0f / 60.0f, 0.0f);
	worldRenderView.setView(view, view);

	// m_worldRenderer->attach(const_cast< Entity* >(rootEntity));
	// m_worldRenderer->setup(worldRenderView, renderGraph);
}

void ProbeCapturer::render(render::IRenderView* renderView, render::ICubeTexture* probeTexture, int32_t face)
{
	T_RENDER_PUSH_MARKER(renderView, "Probe Capture");

	render::Clear clear = { 0 };
	clear.mask = render::CfColor | render::CfDepth;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;

	if (renderView->begin(m_renderTargetSet, &clear))
	{
		//m_worldRenderer->render(renderView, 0);
		renderView->end();
	}

	renderView->copy(
		probeTexture,
		face,
		0,
		m_renderTargetSet->getColorTexture(0),
		0,
		0
	);

	T_RENDER_POP_MARKER(renderView);
}

	}
}
