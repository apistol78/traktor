#include "limits"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/IResourceManager.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Amalgam/Impl/RenderServerEmbedded.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

int32_t sanitizeMultiSample(int32_t multiSample)
{
	switch (multiSample)
	{
	case 1:
		return 0;
	case 2:
		return 2;
	case 3:
		return 2;
	case 4:
		return 4;
	default:
		return 0;
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.RenderServerEmbedded", RenderServerEmbedded, RenderServer)

RenderServerEmbedded::RenderServerEmbedded()
:	m_screenAspectRatio(1.0f)
{
}

bool RenderServerEmbedded::create(Settings* settings, void* nativeWindowHandle)
{
	std::wstring renderType = settings->getProperty< PropertyString >(L"Render.Type");

	Ref< render::IRenderSystem > renderSystem = loadAndInstantiate< render::IRenderSystem >(renderType);
	if (!renderSystem)
		return false;

	render::RenderSystemCreateDesc rscd;
	rscd.windowTitle = L"Puzzle Dimension";
	rscd.mipBias = settings->getProperty< PropertyFloat >(L"Render.MipBias", 0.0f);
	rscd.maxAnisotropy = settings->getProperty< PropertyInteger >(L"Render.MaxAnisotropy", 2);

	if (!renderSystem->create(rscd))
	{
		log::error << L"Render server failed; unable to create render system" << Endl;
		return false;
	}

	m_originalDisplayMode = renderSystem->getCurrentDisplayMode();

	m_screenAspectRatio = renderSystem->getDisplayAspectRatio();
	if (m_screenAspectRatio <= 0.001f)
	{
		if (m_originalDisplayMode.width > 0 && m_originalDisplayMode.height > 0)
			m_screenAspectRatio = float(m_originalDisplayMode.width) / m_originalDisplayMode.height;
		else
		{
			log::warning << L"Unable to get display aspect ratio; assuming square display ratio" << Endl;
			m_screenAspectRatio = 1.0f;
		}
	}

	m_renderViewDesc.depthBits = settings->getProperty< PropertyInteger >(L"Render.DepthBits", 16);
	m_renderViewDesc.stencilBits = settings->getProperty< PropertyInteger >(L"Render.StencilBits", 4);
	m_renderViewDesc.multiSample = settings->getProperty< PropertyInteger >(L"Render.MultiSample", 4);
	m_renderViewDesc.multiSample = sanitizeMultiSample(m_renderViewDesc.multiSample);
	m_renderViewDesc.waitVBlank = settings->getProperty< PropertyBoolean >(L"Render.WaitVBlank", true);
	m_renderViewDesc.nativeWindowHandle = nativeWindowHandle;
	m_renderViewDesc.stereoscopic = false;

	Ref< render::IRenderView > renderView = renderSystem->createRenderView(m_renderViewDesc);
	if (!renderView)
	{
		log::error << L"Render server failed; unable to create render view" << Endl;
		renderSystem->destroy();
		return false;
	}

	// We've successfully created the render view; update settings to reflect found display mode.
	settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", renderView->getWidth());
	settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", renderView->getHeight());
	
	m_renderSystem = renderSystem;
	m_renderView = renderView;

	// Handle some render system messages in order to bring up render view window etc.
	m_renderSystem->handleMessages();

	return true;
}

void RenderServerEmbedded::destroy()
{
	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	safeDestroy(m_renderSystem);
}

void RenderServerEmbedded::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	int32_t skipMips = environment->getSettings()->getProperty< PropertyInteger >(L"Render.SkipMips", 0);
	m_textureFactory = new render::TextureFactory(database, m_renderSystem, skipMips);

	resourceManager->addFactory(m_textureFactory);
	resourceManager->addFactory(new render::ShaderFactory(database, m_renderSystem));
}

int32_t RenderServerEmbedded::reconfigure(const Settings* settings)
{
	int32_t result = CrUnaffected;

	// Update texture quality; manifest through skipping high-detail mips.
	int32_t skipMips = settings->getProperty< PropertyInteger >(L"Render.SkipMips", 0);
	if (skipMips != m_textureFactory->getSkipMips())
	{
		m_textureFactory->setSkipMips(skipMips);
		result |= CrAccepted;
	}

	return result;
}

RenderServer::UpdateResult RenderServerEmbedded::update(Settings* settings)
{
	render::IRenderSystem::HandleResult result = m_renderSystem->handleMessages();
	if (result == render::IRenderSystem::HrTerminate)
		return UrTerminate;

	return UrSuccess;
}

render::IRenderSystem* RenderServerEmbedded::getRenderSystem()
{
	return m_renderSystem;
}

render::IRenderView* RenderServerEmbedded::getRenderView()
{
	return m_renderView;
}

Ref< render::RenderTargetSet > RenderServerEmbedded::createOffscreenTarget(render::TextureFormat format, bool createDepthStencil, bool usingPrimaryDepthStencil)
{
	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = m_renderView->getWidth();
	rtscd.height = m_renderView->getHeight();
	rtscd.multiSample = m_renderViewDesc.multiSample;
	rtscd.createDepthStencil = createDepthStencil;
	rtscd.usingPrimaryDepthStencil = usingPrimaryDepthStencil;
	rtscd.preferTiled = true;
	rtscd.targets[0].format = format;
	return m_renderSystem->createRenderTargetSet(rtscd);
}

float RenderServerEmbedded::getScreenAspectRatio() const
{
	return m_screenAspectRatio;
}

float RenderServerEmbedded::getViewAspectRatio() const
{
	float aspectRatio = float(m_renderView->getWidth()) / m_renderView->getHeight();
	return aspectRatio;
}

float RenderServerEmbedded::getAspectRatio() const
{
	return m_renderView->isFullScreen() ? getScreenAspectRatio() : getViewAspectRatio();
}

bool RenderServerEmbedded::getStereoscopic() const
{
	return m_renderViewDesc.stereoscopic;
}

int32_t RenderServerEmbedded::getMultiSample() const
{
	return m_renderViewDesc.multiSample;
}

	}
}
