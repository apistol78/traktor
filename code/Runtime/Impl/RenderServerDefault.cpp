/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include <limits>
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/RenderServerDefault.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor::runtime
{
	namespace
	{

const float c_maxAspectRatio = 8.0f;

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

int32_t skipMipsFromQuality(int32_t quality)
{
	const int32_t c_skipMips[] =
	{
		3,	// Disabled
		3,	// Low
		2,	// Medium
		1,	// High
		0	// Ultra
	};
	return c_skipMips[quality];
}

int32_t maxAnisotropyFromQuality(int32_t quality)
{
#if defined(__ANDROID__) || defined(__IOS__)
	const int32_t c_maxAnisotropy[] =
	{
		1,	// Disabled
		1,	// Low
		1,	// Medium
		4,	// High
		8	// Ultra
	};
#else
	const int32_t c_maxAnisotropy[] =
	{
		1,	// Disabled
		1,	// Low
		4,	// Medium
		8,	// High
		16	// Ultra
	};
#endif
	return c_maxAnisotropy[quality];
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.RenderServerDefault", RenderServerDefault, RenderServer)

bool RenderServerDefault::create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp)
{
	const std::wstring renderType = defaultSettings->getProperty< std::wstring >(L"Render.Type");
	const std::wstring captureRenderType = settings->getProperty< std::wstring >(L"Render.CaptureType");

	Ref< render::IRenderSystem > renderSystem = dynamic_type_cast< render::IRenderSystem* >(TypeInfo::createInstance(renderType.c_str()));
	if (!renderSystem)
		return false;

	Ref< render::IRenderSystem > captureRenderSystem;
	if (!captureRenderType.empty())
	{
		captureRenderSystem = dynamic_type_cast< render::IRenderSystem* >(TypeInfo::createInstance(captureRenderType.c_str()));
		if (!captureRenderSystem)
			return false;

		std::swap(captureRenderSystem, renderSystem);
	}

	const int32_t textureQuality = settings->getProperty< int32_t >(L"Render.TextureQuality", 2);

	render::RenderSystemDesc rsd;
	rsd.capture = captureRenderSystem;
	rsd.sysapp = sysapp;
	rsd.adapter = settings->getProperty< int32_t >(L"Render.Adapter", -1);
	rsd.mipBias = settings->getProperty< float >(L"Render.MipBias", 0.0f);
	rsd.maxAnisotropy = maxAnisotropyFromQuality(textureQuality);
	rsd.rayTracing = settings->getProperty< bool >(L"Render.RayTracing", false);
	rsd.validation = settings->getProperty< bool >(L"Render.Validation", false);
	rsd.programCache = settings->getProperty< bool >(L"Render.UseProgramCache", true);
	rsd.verbose = true;

	if (!renderSystem->create(rsd))
	{
		log::error << L"Render server failed; unable to create render system." << Endl;
		return false;
	}

	m_renderViewDesc.depthBits = settings->getProperty< int32_t >(L"Render.DepthBits", 24);
	m_renderViewDesc.stencilBits = settings->getProperty< int32_t >(L"Render.StencilBits", 8);
	m_renderViewDesc.multiSample = settings->getProperty< int32_t >(L"Render.MultiSample", 0);
	m_renderViewDesc.allowHDR = settings->getProperty< bool >(L"Render.AllowHDR", true);
	m_renderViewDesc.waitVBlanks = settings->getProperty< int32_t >(L"Render.WaitVBlanks", 1);
	m_renderViewDesc.title = settings->getProperty< std::wstring >(L"Render.Title", L"Traktor");
	m_renderViewDesc.display = (uint32_t)settings->getProperty< int32_t >(L"Render.Display", 0);
	m_renderViewDesc.fullscreen = settings->getProperty< bool >(L"Render.FullScreen", false);

	// Ensure display number are still valid.
	const uint32_t connectedDisplays = renderSystem->getDisplayCount();
	if (m_renderViewDesc.display > connectedDisplays)
	{
		log::warning << L"Configured display " << m_renderViewDesc.display << L" no longer available; " << connectedDisplays << L" connected display(s)." << Endl;
		m_renderViewDesc.display = 0;
	}

	m_originalDisplayMode = renderSystem->getCurrentDisplayMode(m_renderViewDesc.display);

	m_screenAspectRatio = renderSystem->getDisplayAspectRatio(m_renderViewDesc.display);
	if (m_screenAspectRatio <= 0.001f)
	{
		if (m_originalDisplayMode.width > 0 && m_originalDisplayMode.height > 0)
			m_screenAspectRatio = float(m_originalDisplayMode.width) / m_originalDisplayMode.height;
		else
		{
			log::warning << L"Unable to get display aspect ratio; assuming square display ratio." << Endl;
			m_screenAspectRatio = 1.0f;
		}
	}

	// Get display mode from settings; use default settings if none is provided.
	if (m_renderViewDesc.fullscreen)
	{
		m_renderViewDesc.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/Width", m_originalDisplayMode.width);
		m_renderViewDesc.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/Height", m_originalDisplayMode.height);
	}
	else
	{
		const int32_t defaultDenominator = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/DefaultDenominator", 2);
		int32_t defaultWidth = m_originalDisplayMode.width / defaultDenominator;
		int32_t defaultHeight = m_originalDisplayMode.height / defaultDenominator;

		// Clamp width if display is wider than 16:10.
		const float ratio = float(defaultWidth) / defaultHeight;
		if (ratio > 16.0f / 10.0f)
			defaultWidth = (defaultHeight * 16) / 10;

		m_renderViewDesc.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Width", defaultWidth);
		m_renderViewDesc.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Height", defaultHeight);
	}

	m_renderViewDesc.displayMode.colorBits = 24;
	m_renderViewDesc.displayMode.refreshRate = m_originalDisplayMode.refreshRate;

	// Ensure no invalid multi-sample configuration is entered.
	m_renderViewDesc.multiSample = sanitizeMultiSample(m_renderViewDesc.multiSample);

	// Fullscreen is a borderless window covering the display at its current mode;
	// no mode is ever set, so there is no mode to search for.

	Ref< render::IRenderView > renderView = renderSystem->createRenderView(m_renderViewDesc);
	if (!renderView)
	{
		log::error << L"Render server failed; unable to create render view." << Endl;
		renderSystem->destroy();
		return false;
	}

	// Adopt the size the view actually got; in fullscreen the display or the
	// compositor decides it, not the descriptor.
	if (renderView->getWidth() > 0 && renderView->getHeight() > 0)
	{
		m_renderViewDesc.displayMode.width = renderView->getWidth();
		m_renderViewDesc.displayMode.height = renderView->getHeight();
	}

	// We've successfully created the render view; update settings to reflect actual size.
	if (m_renderViewDesc.fullscreen)
	{
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_renderViewDesc.displayMode.width);
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_renderViewDesc.displayMode.height);
	}
	else
	{
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Width", m_renderViewDesc.displayMode.width);
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Height", m_renderViewDesc.displayMode.height);
	}

	m_renderSystem = renderSystem;
	m_renderView = renderView;
	return true;
}

void RenderServerDefault::destroy()
{
	safeClose(m_renderView);
	safeDestroy(m_renderSystem);
}

void RenderServerDefault::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	const int32_t textureQuality = environment->getSettings()->getProperty< int32_t >(L"Render.TextureQuality", 2);
	const int32_t skipMips = skipMipsFromQuality(textureQuality);

	m_textureFactory = new render::TextureFactory(m_renderSystem, skipMips);

	resourceManager->addFactory(m_textureFactory);
}

int32_t RenderServerDefault::reconfigure(IEnvironment* environment, const PropertyGroup* settings)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	int32_t result = CrUnaffected;

	// Must read the same keys with the same defaults as create; else every
	// reconfigure would see a difference and needlessly reset the view.
	render::RenderViewDefaultDesc rvdd;
	rvdd.depthBits = settings->getProperty< int32_t >(L"Render.DepthBits", 24);
	rvdd.stencilBits = settings->getProperty< int32_t >(L"Render.StencilBits", 8);
	rvdd.multiSample = settings->getProperty< int32_t >(L"Render.MultiSample", 0);
	rvdd.allowHDR = settings->getProperty< bool >(L"Render.AllowHDR", true);
	rvdd.waitVBlanks = settings->getProperty< int32_t >(L"Render.WaitVBlanks", 1);
	rvdd.fullscreen = settings->getProperty< bool >(L"Render.FullScreen", false);
	rvdd.title = settings->getProperty< std::wstring >(L"Render.Title", L"Traktor");
	rvdd.display = settings->getProperty< int32_t >(L"Render.Display", 0);

	if (rvdd.fullscreen)
	{
		rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/Width", m_originalDisplayMode.width);
		rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/Height", m_originalDisplayMode.height);
	}
	else
	{
		rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Width", m_originalDisplayMode.width / 2);
		rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Height", m_originalDisplayMode.height / 2);
	}

	rvdd.displayMode.colorBits = 24;
	rvdd.displayMode.refreshRate = m_originalDisplayMode.refreshRate;

	// Ensure no invalid multi-sample configuration is entered.
	rvdd.multiSample = sanitizeMultiSample(rvdd.multiSample);

	// Fullscreen is a borderless window covering the display at its current mode;
	// no mode is ever set, so there is no mode to search for.

	// Check if we need to reset render view. The stored descriptor always carries the
	// size the view actually got, so a mode change which the display or compositor
	// resolved differently doesn't come back around as another reset.
	if (
		m_renderViewDesc.depthBits != rvdd.depthBits ||
		m_renderViewDesc.stencilBits != rvdd.stencilBits ||
		m_renderViewDesc.multiSample != rvdd.multiSample ||
		m_renderViewDesc.waitVBlanks != rvdd.waitVBlanks ||
		m_renderViewDesc.fullscreen != rvdd.fullscreen ||
		m_renderViewDesc.displayMode.width != rvdd.displayMode.width ||
		m_renderViewDesc.displayMode.height != rvdd.displayMode.height
	)
	{
		T_DEBUG(L"Render view settings changed; resetting view...");
		render::RenderViewDefaultDesc current = m_renderViewDesc;

		// Reset primary render view, restore descriptor if fail.
		m_renderViewDesc = rvdd;
		if (!m_renderView->reset(rvdd))
		{
			log::error << L"Failed to apply changes to render view; current is kept" << Endl;
			m_renderViewDesc = current;
			return CrFailed;
		}

		// Adopt the size the view actually got; in fullscreen the display or the
		// compositor decides it, not the descriptor.
		if (m_renderView->getWidth() > 0 && m_renderView->getHeight() > 0)
		{
			m_renderViewDesc.displayMode.width = m_renderView->getWidth();
			m_renderViewDesc.displayMode.height = m_renderView->getHeight();
		}

		result = CrAccepted;
	}
	else
		T_DEBUG(L"Render view settings unchanged");

	const int32_t textureQuality = settings->getProperty< int32_t >(L"Render.TextureQuality", 2);

	// Update texture quality; manifest through skipping high-detail mips.
	const int32_t skipMips = skipMipsFromQuality(textureQuality);
	if (skipMips != m_textureFactory->getSkipMips())
	{
		m_textureFactory->setSkipMips(skipMips);

		// Unload all texture resources from resource manager.
		resourceManager->unload(type_of< render::ITexture >());

		// Ensure all unloaded textures are purged; ie totally evicted from video memory.
		m_renderSystem->purge();

		// Reload all texture resources; this time using new skip mips setting.
		resourceManager->reload(type_of< render::ITexture >(), false);
		result |= CrAccepted;
	}

	// Reset render system.
	render::RenderSystemDesc rsd;
	rsd.adapter = settings->getProperty< int32_t >(L"Render.Adapter", -1);
	rsd.mipBias = settings->getProperty< float >(L"Render.MipBias", 0.0f);
	rsd.maxAnisotropy = maxAnisotropyFromQuality(textureQuality);
	if (!m_renderSystem->reset(rsd))
		return CrFailed;

	return result;
}

RenderServer::UpdateResult RenderServerDefault::update(PropertyGroup* settings)
{
	RenderServer::update(settings);

	if (!m_renderView)
		return UrSuccess;

	bool fullscreen = m_renderView->isFullScreen();
	bool reconfigure = false;

	render::RenderEvent evt;
	while (m_renderView->nextEvent(evt))
	{
		if (evt.type == render::RenderEventType::Close)
			return UrTerminate;
		else if (evt.type == render::RenderEventType::ToggleFullScreen)
		{
			fullscreen = !fullscreen;
			reconfigure = true;
		}
		else if (evt.type == render::RenderEventType::SetWindowed)
		{
			fullscreen = false;
			reconfigure = true;
		}
		else if (evt.type == render::RenderEventType::SetFullScreen)
		{
			fullscreen = true;
			reconfigure = true;
		}
		else if (evt.type == render::RenderEventType::Resize)
		{
			if (m_renderView->isFullScreen())
			{
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", evt.resize.width);
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", evt.resize.height);
			}
			else
			{
				settings->setProperty< PropertyInteger >(L"Render.Display", m_renderView->getDisplay());
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Width", evt.resize.width);
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Height", evt.resize.height);
			}
			reconfigure = true;
		}
		else if (evt.type == render::RenderEventType::Lost)
		{
			m_renderViewDesc.displayMode.width = 0;
			m_renderViewDesc.displayMode.height = 0;
			reconfigure = true;
		}
	}

	if (reconfigure)
	{
		if (fullscreen != settings->getProperty< bool >(L"Render.FullScreen", false))
			settings->setProperty< PropertyBoolean >(L"Render.FullScreen", fullscreen);
		return UrReconfigure;
	}

	return UrSuccess;
}

render::IRenderSystem* RenderServerDefault::getRenderSystem()
{
	return m_renderSystem;
}

render::IRenderView* RenderServerDefault::getRenderView()
{
	return m_renderView;
}

int32_t RenderServerDefault::getWidth() const
{
	return m_renderView->getWidth();
}

int32_t RenderServerDefault::getHeight() const
{
	return m_renderView->getHeight();
}

float RenderServerDefault::getScreenAspectRatio() const
{
	return m_screenAspectRatio;
}

float RenderServerDefault::getViewAspectRatio() const
{
	const float aspectRatio = float(getWidth()) / getHeight();
	return min(aspectRatio, c_maxAspectRatio);
}

float RenderServerDefault::getAspectRatio() const
{
	return getViewAspectRatio();
}

int32_t RenderServerDefault::getMultiSample() const
{
	return m_renderViewDesc.multiSample;
}

float RenderServerDefault::getRefreshRate() const
{
	return m_renderViewDesc.displayMode.refreshRate;
}

}
