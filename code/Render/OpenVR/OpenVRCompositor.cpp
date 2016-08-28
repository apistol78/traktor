#include <openvr.h>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/OpenVR/OpenVRCompositor.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.OpenVRCompositor", 0, OpenVRCompositor, IVRCompositor)

OpenVRCompositor::OpenVRCompositor()
:	m_vr(0)
{
}

bool OpenVRCompositor::create(IRenderSystem* renderSystem, IRenderView* renderView)
{
	vr::EVRInitError err = vr::VRInitError_None;

	// Initialize OpenVR.
	m_vr = vr::VR_Init(&err, vr::VRApplication_Scene);
	if (err != vr::VRInitError_None)
	{
		log::error << L"Failed to initialize OpenVR compositor; VR_Init failed \"" << mbstows(vr::VR_GetVRInitErrorAsEnglishDescription(err)) << L"\"." << Endl;
		return false;
	}

	// Initialize OpenVR own compositor.
	if (!vr::VRCompositor())
	{
		log::error << L"Failed to initialize OpenVR compositor; VRCompositor failed." << Endl;
		return false;
	}

	int32_t adapterIndex = 0;
	m_vr->GetDXGIOutputInfo(&adapterIndex);
	log::info << L"OpenVR; Should use adapter " << adapterIndex << L" for DX11 initialization." << Endl;

	uint32_t width = 0, height = 0;
	m_vr->GetRecommendedRenderTargetSize(&width, &height);
	if (!width || !height)
	{
		log::error << L"Failed to initialize OpenVR compositor; Unable to determine intermediate render target size." << Endl;
		return false;
	}

	log::info << L"OpenVR; Using intermediate render target size " << width << L" * " << height << Endl;

	// Create render target set.
	RenderTargetSetCreateDesc rtscd;
	rtscd.count = 2;
	rtscd.width = width;
	rtscd.height = height;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = true;
	rtscd.usingDepthStencilAsTexture = false;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.preferTiled = false;
	rtscd.ignoreStencil = false;
	rtscd.generateMips = false;
	rtscd.targets[0].format = TfR8G8B8A8;
	rtscd.targets[0].sRGB = false;
	rtscd.targets[1].format = TfR8G8B8A8;
	rtscd.targets[1].sRGB = false;
	if ((m_targetSet = renderSystem->createRenderTargetSet(rtscd)) == 0)
	{
		log::error << L"Failed to initialize OpenVR compositor; Unable to create render targets." << Endl;
		return false;
	}

	return true;
}

void OpenVRCompositor::destroy()
{
	safeDestroy(m_targetSet);

	if (m_vr)
	{
		vr::VR_Shutdown();
		m_vr = 0;
	}
}

int32_t OpenVRCompositor::getWidth() const
{
	return m_targetSet->getWidth();
}

int32_t OpenVRCompositor::getHeight() const
{
	return m_targetSet->getHeight();
}

bool OpenVRCompositor::beginRenderEye(IRenderView* renderView, int32_t eye)
{
	if (!renderView->begin(m_targetSet, eye == EtLeft ? 0 : 1))
		return false;

	const Color4f clearColor(0.0f, 0.0f, 0.0f, 0.0f);
	renderView->clear(
		render::CfColor | render::CfDepth | render::CfStencil,
		&clearColor,
		1.0f,
		0
	);

	return true;
}

bool OpenVRCompositor::endRenderEye(IRenderView* renderView, int32_t eye)
{
	renderView->end();
	return true;
}

bool OpenVRCompositor::presentCompositeOutput(IRenderView* renderView)
{
	vr::VREvent_t event;
	while (m_vr->PollNextEvent(&event, sizeof(event)))
	{
		//ProcessVREvent(event);
	}

	vr::Texture_t leftEyeTexture =
	{
		m_targetSet->getColorTexture(0)->getInternalHandle(),
		vr::API_DirectX,
		vr::ColorSpace_Gamma
	};
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

	vr::Texture_t rightEyeTexture =
	{
		m_targetSet->getColorTexture(1)->getInternalHandle(),
		vr::API_DirectX,
		vr::ColorSpace_Gamma
	};
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	renderView->present();

	vr::VRCompositor()->WaitGetPoses(NULL, 0, NULL, 0);

	//vr::VRCompositor()->PostPresentHandoff();
	return true;
}

	}
}
