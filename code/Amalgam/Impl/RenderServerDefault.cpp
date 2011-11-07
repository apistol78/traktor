#include "limits"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
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
#include "Amalgam/Impl/RenderServerDefault.h"

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

bool findDisplayMode(render::IRenderSystem* renderSystem, const render::DisplayMode& criteria, render::DisplayMode& outBestMatch)
{
	int32_t bestMatch = std::numeric_limits< int32_t >::max();
	uint32_t bestDisplayModeIndex = 0;

	uint32_t displayModeCount = renderSystem->getDisplayModeCount();
	if (!displayModeCount)
	{
		render::DisplayMode currentMode = renderSystem->getCurrentDisplayMode();
		if (currentMode.width != 0 && currentMode.height != 0)
		{
			log::warning << L"Unable to enumerate display modes; using current display mode as fail safe" << Endl;
			outBestMatch = currentMode;
		}
		else
		{
			log::warning << L"Unable to enumerate display modes; using criteria display mode as fail safe" << Endl;
			outBestMatch = criteria;
		}
		return true;
	}
	
	const uint32_t c_preferColorBits[] = { 24, 32, 16, 15 };
	for (uint32_t i = 0; i < sizeof_array(c_preferColorBits); ++i)
	{
		for (uint32_t j = 0; j < displayModeCount; ++j)
		{
			render::DisplayMode check = renderSystem->getDisplayMode(j);
			if (
				check.colorBits != c_preferColorBits[i] ||
				check.stereoscopic != criteria.stereoscopic
			)
				continue;
		
			int32_t match =
				std::abs((int32_t)(check.width - criteria.width)) +
				std::abs((int32_t)(check.height - criteria.height));

			if (match < bestMatch)
			{
				bestDisplayModeIndex = j;
				bestMatch = match;
			}
		}
		if (bestMatch == 0)
			break;
	}

	if (bestMatch != std::numeric_limits< int32_t >::max())
	{
		outBestMatch = renderSystem->getDisplayMode(bestDisplayModeIndex);
	}
	else
	{
		log::warning << L"Unable to find matching display mode; using current display mode as fail safe" << Endl;
		outBestMatch = renderSystem->getCurrentDisplayMode();
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.RenderServerDefault", RenderServerDefault, RenderServer)

RenderServerDefault::RenderServerDefault()
:	m_screenAspectRatio(1.0f)
{
}

bool RenderServerDefault::create(Settings* settings)
{
	std::wstring renderType = settings->getProperty< PropertyString >(L"Render.Type");

	Ref< render::IRenderSystem > renderSystem = loadAndInstantiate< render::IRenderSystem >(renderType);
	if (!renderSystem)
		return false;

	render::RenderSystemCreateDesc rscd;
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
	m_renderViewDesc.waitVBlank = settings->getProperty< PropertyBoolean >(L"Render.WaitVBlank", true);
	m_renderViewDesc.title = L"Puzzle Dimension";
	m_renderViewDesc.fullscreen = settings->getProperty< PropertyBoolean >(L"Render.FullScreen", false);

#if defined(_PS3)

	if (!m_originalDisplayMode.stereoscopic)
	{
		switch (m_originalDisplayMode.height)
		{
		case 720:
			log::info << L"Using HD television settings" << Endl;
			m_renderViewDesc.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionHD/Width", 1280);
			m_renderViewDesc.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionHD/Height", 720);
			m_renderViewDesc.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionHD/MultiSample", 0);
			break;

		case 1080:
			log::info << L"Using FullHD television settings" << Endl;
			m_renderViewDesc.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionFullHD/Width", 1440);
			m_renderViewDesc.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionFullHD/Height", 1080);
			m_renderViewDesc.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionFullHD/MultiSample", 0);
			break;

		default:
			log::info << L"Using default television settings" << Endl;
			m_renderViewDesc.displayMode.width = m_originalDisplayMode.width;
			m_renderViewDesc.displayMode.height = m_originalDisplayMode.height;
			m_renderViewDesc.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionStandard/MultiSample", 0);
		}
		m_renderViewDesc.displayMode.stereoscopic = false;
	}
	else
	{
		log::info << L"Using 3D television settings" << Endl;
		m_renderViewDesc.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Television3D/Width", 960);
		m_renderViewDesc.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Television3D/Height", 720);
		m_renderViewDesc.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Television3D/MultiSample", 0);
		m_renderViewDesc.displayMode.stereoscopic = true;
	}
	m_renderViewDesc.displayMode.colorBits = 24;

#else

	// Get display mode from settings; use default settings if none is provided.
	if (m_renderViewDesc.fullscreen)
	{
		m_renderViewDesc.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_originalDisplayMode.width);
		m_renderViewDesc.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_originalDisplayMode.height);
	}
	else
	{
		m_renderViewDesc.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode.Window/Width", m_originalDisplayMode.width / 2);
		m_renderViewDesc.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode.Window/Height", m_originalDisplayMode.height / 2);
	}

	m_renderViewDesc.displayMode.stereoscopic = settings->getProperty< PropertyBoolean >(L"Render.Stereoscopic", false);
	m_renderViewDesc.displayMode.colorBits = 24;

#endif

	// Ensure no invalid multi-sample configuration is entered.
	m_renderViewDesc.multiSample = sanitizeMultiSample(m_renderViewDesc.multiSample);

	// Ensure display mode is still supported; else find closest match.
	if (m_renderViewDesc.fullscreen)
	{
		if (!findDisplayMode(renderSystem, m_renderViewDesc.displayMode, m_renderViewDesc.displayMode))
		{
			log::error << L"Render server failed; unable to find an acceptable display mode" << Endl;
			renderSystem->destroy();
			return false;
		}
	}

	Ref< render::IRenderView > renderView = renderSystem->createRenderView(m_renderViewDesc);
	if (!renderView)
	{
		log::error << L"Render server failed; unable to create render view" << Endl;
		renderSystem->destroy();
		return false;
	}
	
	// We've successfully created the render view; update settings to reflect found display mode.
#if !defined(_PS3)
	if (m_renderViewDesc.fullscreen)
	{
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_renderViewDesc.displayMode.width);
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_renderViewDesc.displayMode.height);
	}
#endif

	settings->setProperty< PropertyBoolean >(L"Render.Stereoscopic", m_renderViewDesc.displayMode.stereoscopic);
	
	m_renderSystem = renderSystem;
	m_renderView = renderView;

	return true;
}

void RenderServerDefault::destroy()
{
	if (m_renderView)
	{
		m_renderView->close();
		m_renderView = 0;
	}

	safeDestroy(m_renderSystem);
}

void RenderServerDefault::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	int32_t skipMips = environment->getSettings()->getProperty< PropertyInteger >(L"Render.SkipMips", 0);
	m_textureFactory = new render::TextureFactory(database, m_renderSystem, skipMips);

	resourceManager->addFactory(m_textureFactory);
	resourceManager->addFactory(new render::ShaderFactory(database, m_renderSystem));
}

int32_t RenderServerDefault::reconfigure(const Settings* settings)
{
	int32_t result = CrUnaffected;

	render::RenderViewDefaultDesc rvdd;
	rvdd.depthBits = settings->getProperty< PropertyInteger >(L"Render.DepthBits", 16);
	rvdd.stencilBits = settings->getProperty< PropertyInteger >(L"Render.StencilBits", 8);
	rvdd.multiSample = settings->getProperty< PropertyInteger >(L"Render.MultiSample", 4);
	rvdd.waitVBlank = settings->getProperty< PropertyBoolean >(L"Render.WaitVBlank", true);
	rvdd.fullscreen = settings->getProperty< PropertyBoolean >(L"Render.FullScreen", false);
	rvdd.title = L"Puzzle Dimension";

#if defined(_PS3)

	rvdd.displayMode.stereoscopic = settings->getProperty< PropertyBoolean >(L"Render.Stereoscopic", false);
	if (!rvdd.displayMode.stereoscopic)
	{
		switch (m_originalDisplayMode.height)
		{
		case 720:
			log::info << L"Using HD television settings" << Endl;
			rvdd.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionHD/Width", 1280);
			rvdd.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionHD/Height", 720);
			rvdd.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionHD/MultiSample", 0);
			break;

		case 1080:
			log::info << L"Using FullHD television settings" << Endl;
			rvdd.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionFullHD/Width", 1440);
			rvdd.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionFullHD/Height", 1080);
			rvdd.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionFullHD/MultiSample", 0);
			break;

		default:
			log::info << L"Using default television settings" << Endl;
			rvdd.displayMode.width = m_originalDisplayMode.width;
			rvdd.displayMode.height = m_originalDisplayMode.height;
			rvdd.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/TelevisionStandard/MultiSample", 0);
		}
	}
	else
	{
		log::info << L"Using 3D television settings" << Endl;
		rvdd.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Television3D/Width", 960);
		rvdd.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Television3D/Height", 720);
		rvdd.multiSample = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Television3D/MultiSample", 0);
	}
	rvdd.displayMode.colorBits = 24;

#else

	if (rvdd.fullscreen)
	{
		rvdd.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_originalDisplayMode.width);
		rvdd.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_originalDisplayMode.height);
	}
	else
	{
		rvdd.displayMode.width = settings->getProperty< PropertyInteger >(L"Render.DisplayMode.Window/Width", m_originalDisplayMode.width / 2);
		rvdd.displayMode.height = settings->getProperty< PropertyInteger >(L"Render.DisplayMode.Window/Height", m_originalDisplayMode.height / 2);
	}

	rvdd.displayMode.stereoscopic = settings->getProperty< PropertyBoolean >(L"Render.Stereoscopic", false);
	rvdd.displayMode.colorBits = 24;

#endif

	// Ensure display mode is still supported; else find closest match.
	if (rvdd.fullscreen)
	{
		if (!findDisplayMode(m_renderSystem, rvdd.displayMode, rvdd.displayMode))
		{
			log::error << L"Unable to find an acceptable display mode; unable to continue" << Endl;
			return CrFailed;
		}
	}

	// Check if we need to reset render view.
	if (
		m_renderViewDesc.depthBits != rvdd.depthBits ||
		m_renderViewDesc.stencilBits != rvdd.stencilBits ||
		m_renderViewDesc.multiSample != rvdd.multiSample ||
		m_renderViewDesc.waitVBlank != rvdd.waitVBlank ||
		m_renderViewDesc.fullscreen != rvdd.fullscreen ||
		m_renderViewDesc.displayMode.width != rvdd.displayMode.width ||
		m_renderViewDesc.displayMode.height != rvdd.displayMode.height ||
		m_renderViewDesc.displayMode.stereoscopic != rvdd.displayMode.stereoscopic
	)
	{
		// Reset primary render view.
		if (!m_renderView->reset(rvdd))
			return CrFailed;

		m_renderViewDesc = rvdd;
		result = CrAccepted;
	}

	// Update texture quality; manifest through skipping high-detail mips.
	int32_t skipMips = settings->getProperty< PropertyInteger >(L"Render.SkipMips", 0);
	if (skipMips != m_textureFactory->getSkipMips())
	{
		m_textureFactory->setSkipMips(skipMips);
		result |= CrAccepted | CrFlushResources;
	}

	return result;
}

RenderServer::UpdateResult RenderServerDefault::update(Settings* settings)
{
#if !defined(_PS3)

	if (!m_renderView)
		return UrSuccess;

	render::RenderEvent evt;
	if (m_renderView->nextEvent(evt))
	{
		if (evt.type == render::ReClose)
			return UrTerminate;
		else if (evt.type == render::ReToggleFullScreen)
		{
			settings->setProperty< PropertyBoolean >(L"Render.FullScreen", !m_renderViewDesc.fullscreen);
			return UrReconfigure;
		}
		else if (evt.type == render::ReResize)
		{
			if (!m_renderViewDesc.fullscreen)
			{
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Width", evt.resize.width);
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Height", evt.resize.height);
			}
			return UrReconfigure;
		}
	}

	return UrSuccess;

#else

	// Reflect gamma real-time as we need to preview gamma changes before
	// they are applied.
	if (m_renderView)
	{
		float gamma = settings->getProperty< PropertyFloat >(L"Render.Gamma", 1.0f);

		// \fixme gamma Inverted and from 0.5 to 1.5 range.
		m_renderView->setGamma(lerp(1.2f, 0.8f, gamma - 0.5f));
	}

	return UrSuccess;

#endif
}

render::IRenderSystem* RenderServerDefault::getRenderSystem()
{
	return m_renderSystem;
}

render::IRenderView* RenderServerDefault::getRenderView()
{
	return m_renderView;
}

Ref< render::RenderTargetSet > RenderServerDefault::createOffscreenTarget(render::TextureFormat format, bool createDepthStencil, bool usingPrimaryDepthStencil)
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

float RenderServerDefault::getScreenAspectRatio() const
{
	return m_screenAspectRatio;
}

float RenderServerDefault::getViewAspectRatio() const
{
	float aspectRatio = float(m_renderView->getWidth()) / m_renderView->getHeight();
	if (aspectRatio < 3.0f)
		return aspectRatio;
	else
		return 3.0f;
}

float RenderServerDefault::getAspectRatio() const
{
	return m_renderView->isFullScreen() ? getScreenAspectRatio() : getViewAspectRatio();
}

bool RenderServerDefault::getStereoscopic() const
{
	return m_renderViewDesc.displayMode.stereoscopic;
}

int32_t RenderServerDefault::getMultiSample() const
{
	return m_renderViewDesc.multiSample;
}

	}
}
