#include "Amalgam/IEnvironment.h"
#include "Amalgam/Action/AsConfiguration.h"
#include "Amalgam/Action/AsDisplayMode.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.AsConfiguration", AsConfiguration, flash::ActionObjectRelay)

AsConfiguration::AsConfiguration()
:	flash::ActionObjectRelay("traktor.amalgam.Configuration")
,	m_displayModeWidth(1280)
,	m_displayModeHeight(720)
,	m_fullscreen(false)
,	m_waitVBlank(true)
,	m_multiSample(0)
,	m_gamma(2.0f)
,	m_stereoscopic(false)
,	m_textureQuality(QtMedium)
,	m_shadowQuality(QtMedium)
,	m_ambientOcclusionQuality(QtMedium)
,	m_antiAliasQuality(QtMedium)
,	m_rumbleEnable(true)
,	m_masterVolume(1.0f)
{
}

Ref< AsConfiguration > AsConfiguration::getCurrent(amalgam::IEnvironment* environment)
{
	const PropertyGroup* settings = environment->getSettings();
	T_ASSERT (settings);

	Ref< AsConfiguration > current = new AsConfiguration();
	
	current->m_displayModeWidth = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Width", 1280);
	current->m_displayModeHeight = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Height", 720);
	current->m_fullscreen = settings->getProperty< PropertyBoolean >(L"Render.FullScreen", false);
	current->m_waitVBlank = settings->getProperty< PropertyBoolean >(L"Render.WaitVBlank", true);
	current->m_multiSample = settings->getProperty< PropertyInteger >(L"Render.MultiSample", 0);
	current->m_gamma = settings->getProperty< PropertyFloat >(L"Render.Gamma", 2.0f);
	current->m_stereoscopic = settings->getProperty< PropertyBoolean >(L"Render.Stereoscopic", false);
	current->m_textureQuality = (Quality)settings->getProperty< PropertyInteger >(L"Render.TextureQuality", QtMedium);
	current->m_shadowQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.ShadowQuality", QtMedium);
	current->m_ambientOcclusionQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", QtMedium);
	current->m_antiAliasQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.AntiAliasQuality", QtMedium);
	current->m_rumbleEnable = settings->getProperty< PropertyBoolean >(L"Input.Rumble", true);
	current->m_masterVolume = settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f);

	Ref< const PropertyGroup > volumes = settings->getProperty< PropertyGroup >(L"Audio.Volumes");
	if (volumes)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& cv = volumes->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = cv.begin(); i != cv.end(); ++i)
		{
			const std::wstring& category = i->first;
			float volume = PropertyFloat::get(i->second);
			current->m_volumes.insert(std::make_pair(category, volume));
		}
	}

	return current;
}

Ref< AsDisplayMode > AsConfiguration::getDisplayMode() const
{
	render::DisplayMode dm;
	dm.width = m_displayModeWidth;
	dm.height = m_displayModeHeight;
	return new AsDisplayMode(dm);
}

void AsConfiguration::setDisplayMode(const AsDisplayMode* displayMode)
{
	const render::DisplayMode& dm = displayMode->getDisplayMode();
	m_displayModeWidth = dm.width;
	m_displayModeHeight = dm.height;
}

bool AsConfiguration::getFullscreen() const
{
	return m_fullscreen;
}

void AsConfiguration::setFullscreen(bool fullscreen)
{
	m_fullscreen = fullscreen;
}

bool AsConfiguration::getWaitVBlank() const
{
	return m_waitVBlank;
}

void AsConfiguration::setWaitVBlank(bool waitVBlank)
{
	m_waitVBlank = waitVBlank;
}

int32_t AsConfiguration::getMultiSample() const
{
	return m_multiSample;
}

void AsConfiguration::setMultiSample(int32_t multiSample)
{
	m_multiSample = multiSample;
}

float AsConfiguration::getGamma() const
{
	return m_gamma;
}

void AsConfiguration::setGamma(float gamma)
{
	m_gamma = gamma;
}

bool AsConfiguration::getStereoscopic() const
{
	return m_stereoscopic;
}

void AsConfiguration::setStereoscopic(bool stereoscopic)
{
	m_stereoscopic = stereoscopic;
}

AsConfiguration::Quality AsConfiguration::getTextureQuality() const
{
	return m_textureQuality;
}

void AsConfiguration::setTextureQuality(Quality textureQuality)
{
	m_textureQuality = textureQuality;
}

AsConfiguration::Quality AsConfiguration::getShadowQuality() const
{
	return m_shadowQuality;
}

void AsConfiguration::setShadowQuality(Quality shadowQuality)
{
	m_shadowQuality = shadowQuality;
}

AsConfiguration::Quality AsConfiguration::getAmbientOcclusionQuality() const
{
	return m_ambientOcclusionQuality;
}

void AsConfiguration::setAmbientOcclusionQuality(Quality ambientOcclusionQuality)
{
	m_ambientOcclusionQuality = ambientOcclusionQuality;
}

AsConfiguration::Quality AsConfiguration::getAntiAliasQuality() const
{
	return m_antiAliasQuality;
}

void AsConfiguration::setAntiAliasQuality(Quality antiAliasQuality)
{
	m_antiAliasQuality = antiAliasQuality;
}

bool AsConfiguration::getRumbleEnable() const
{
	return m_rumbleEnable;
}

void AsConfiguration::setRumbleEnable(bool rumbleEnable)
{
	m_rumbleEnable = rumbleEnable;
}

float AsConfiguration::getVolume() const
{
	return m_masterVolume;
}

void AsConfiguration::setVolume(float volume)
{
	m_masterVolume = volume;
}

float AsConfiguration::getVolume(const std::wstring& category) const
{
	std::map< std::wstring, float >::const_iterator i = m_volumes.find(category);
	return i != m_volumes.end() ? i->second : 1.0f;
}

void AsConfiguration::setVolume(const std::wstring& category, float volume)
{
	m_volumes[category] = volume;
}

bool AsConfiguration::apply(amalgam::IEnvironment* environment)
{
	PropertyGroup* settings = environment->getSettings();
	T_ASSERT (settings);

	settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_displayModeWidth);
	settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_displayModeHeight);
	settings->setProperty< PropertyBoolean >(L"Render.FullScreen", m_fullscreen);
	settings->setProperty< PropertyBoolean >(L"Render.WaitVBlank", m_waitVBlank);
	settings->setProperty< PropertyInteger >(L"Render.MultiSample", m_multiSample);
	settings->setProperty< PropertyFloat >(L"Render.Gamma", m_gamma);
	settings->setProperty< PropertyBoolean >(L"Render.Stereoscopic", m_stereoscopic);
	settings->setProperty< PropertyInteger >(L"Render.TextureQuality", m_textureQuality);
	settings->setProperty< PropertyInteger >(L"World.ShadowQuality", m_shadowQuality);
	settings->setProperty< PropertyInteger >(L"World.AmbientOcclusionQuality", m_ambientOcclusionQuality);
	settings->setProperty< PropertyInteger >(L"World.AntiAliasQuality", m_antiAliasQuality);
	settings->setProperty< PropertyBoolean >(L"Input.Rumble", m_rumbleEnable);
	settings->setProperty< PropertyFloat >(L"Audio.MasterVolume", m_masterVolume);

	for (std::map< std::wstring, float >::const_iterator i = m_volumes.begin(); i != m_volumes.end(); ++i)
		settings->setProperty< PropertyFloat >(L"Audio.Volumes/" + i->first, i->second);

	return environment->reconfigure();
}

	}
}
