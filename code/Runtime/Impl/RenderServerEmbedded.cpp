/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/RenderServerEmbedded.h"
#include "Core/Log/Log.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.RenderServerEmbedded", RenderServerEmbedded, RenderServer)

bool RenderServerEmbedded::create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp, const SystemWindow& syswin)
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
		log::error << L"Render server failed; unable to create render system" << Endl;
		return false;
	}

	m_originalDisplayMode = renderSystem->getCurrentDisplayMode(0);

	m_screenAspectRatio = renderSystem->getDisplayAspectRatio(0);
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

	m_renderViewDesc.depthBits = settings->getProperty< int32_t >(L"Render.DepthBits", 24);
	m_renderViewDesc.stencilBits = settings->getProperty< int32_t >(L"Render.StencilBits", 8);
	m_renderViewDesc.multiSample = settings->getProperty< int32_t >(L"Render.MultiSample", 4);
	m_renderViewDesc.multiSample = sanitizeMultiSample(m_renderViewDesc.multiSample);
	m_renderViewDesc.waitVBlanks = settings->getProperty< int32_t >(L"Render.WaitVBlanks", 1);
	m_renderViewDesc.syswin = syswin;

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

	return true;
}

void RenderServerEmbedded::destroy()
{
	safeClose(m_renderView);
	safeDestroy(m_renderSystem);
}

void RenderServerEmbedded::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	const int32_t textureQuality = environment->getSettings()->getProperty< int32_t >(L"Render.TextureQuality", 2);
	const int32_t skipMips = skipMipsFromQuality(textureQuality);

	m_textureFactory = new render::TextureFactory(m_renderSystem, skipMips);

	resourceManager->addFactory(m_textureFactory);
}

int32_t RenderServerEmbedded::reconfigure(IEnvironment* environment, const PropertyGroup* settings)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	int32_t result = CrUnaffected;

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

RenderServer::UpdateResult RenderServerEmbedded::update(PropertyGroup* settings)
{
	RenderServer::update(settings);

	if (!m_renderView)
		return UrSuccess;

	render::RenderEvent evt;
	while (m_renderView->nextEvent(evt))
	{
		if (evt.type == render::RenderEventType::Close)
			return UrTerminate;
		else if (evt.type == render::RenderEventType::Resize)
			return UrReconfigure;
	}

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

int32_t RenderServerEmbedded::getWidth() const
{
	return m_renderView->getWidth();
}

int32_t RenderServerEmbedded::getHeight() const
{
	return m_renderView->getHeight();
}

float RenderServerEmbedded::getScreenAspectRatio() const
{
	return m_screenAspectRatio;
}

float RenderServerEmbedded::getViewAspectRatio() const
{
	const float aspectRatio = float(m_renderView->getWidth()) / m_renderView->getHeight();
	return aspectRatio;
}

float RenderServerEmbedded::getAspectRatio() const
{
	return m_renderView->isFullScreen() ? getScreenAspectRatio() : getViewAspectRatio();
}

int32_t RenderServerEmbedded::getMultiSample() const
{
	return m_renderViewDesc.multiSample;
}

float RenderServerEmbedded::getRefreshRate() const
{
	return m_originalDisplayMode.refreshRate;
}

}
