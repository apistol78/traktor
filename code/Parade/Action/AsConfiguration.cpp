#include "Amalgam/IEnvironment.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Parade/Action/AsConfiguration.h"
#include "Parade/Action/AsDisplayMode.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.AsConfiguration", AsConfiguration, flash::ActionObjectRelay)

AsConfiguration::AsConfiguration()
:	flash::ActionObjectRelay("traktor.parade.Configuration")
,	m_fullscreen(false)
,	m_waitVBlank(true)
,	m_multiSample(0)
,	m_gamma(2.0f)
,	m_stereoscopic(false)
,	m_textureQuality(QtMedium)
,	m_shadowQuality(QtMedium)
,	m_masterVolume(1.0f)
,	m_ambientVolume(1.0f)
,	m_soundFxVolume(1.0f)
,	m_musicVolume(1.0f)
,	m_rumbleEnable(true)
{
}

Ref< AsConfiguration > AsConfiguration::getCurrent(amalgam::IEnvironment* environment)
{
	const PropertyGroup* settings = environment->getSettings();
	T_ASSERT (settings);

	Ref< AsConfiguration > current = new AsConfiguration();
	//current->m_displayModeWidth = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Width", 1280);
	//current->m_displayModeHeight = settings->getProperty< PropertyInteger >(L"Render.DisplayMode/Height", 720);
	current->m_fullscreen = settings->getProperty< PropertyBoolean >(L"Render.FullScreen", false);
	current->m_waitVBlank = settings->getProperty< PropertyBoolean >(L"Render.WaitVBlank", true);
	current->m_multiSample = settings->getProperty< PropertyInteger >(L"Render.MultiSample", 0);
	current->m_gamma = settings->getProperty< PropertyFloat >(L"Render.Gamma", 2.0f);
	current->m_stereoscopic = settings->getProperty< PropertyBoolean >(L"Render.Stereoscopic", false);
	current->m_textureQuality = (Quality)settings->getProperty< PropertyInteger >(L"Render.TextureQuality", QtMedium);
	current->m_shadowQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.ShadowQuality", QtMedium);
	current->m_masterVolume = settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f);
	current->m_ambientVolume = settings->getProperty< PropertyFloat >(L"Audio.AmbientVolume", 1.0f);
	current->m_soundFxVolume = settings->getProperty< PropertyFloat >(L"Audio.SoundFxVolume", 1.0f);
	current->m_musicVolume = settings->getProperty< PropertyFloat >(L"Audio.MusicVolume", 1.0f);
	current->m_rumbleEnable = settings->getProperty< PropertyBoolean >(L"Input.Rumble", true);

	return current;
}

Ref< AsDisplayMode > AsConfiguration::getDisplayMode() const
{
	Ref< AsDisplayMode > actionDisplayMode = new AsDisplayMode();
	//actionDisplayMode->dm.width = m_displayModeWidth;
	//actionDisplayMode->dm.height = m_displayModeHeight;
	return actionDisplayMode;
}

void AsConfiguration::setDisplayMode(const AsDisplayMode* displayMode)
{
	//m_displayModeWidth = displayMode->dm.width;
	//m_displayModeHeight = displayMode->dm.height;
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

float AsConfiguration::getMasterVolume() const
{
	return m_masterVolume;
}

void AsConfiguration::setMasterVolume(float masterVolume)
{
	m_masterVolume = masterVolume;
}

float AsConfiguration::getAmbientVolume() const
{
	return m_ambientVolume;
}

void AsConfiguration::setAmbientVolume(float ambientVolume)
{
	m_ambientVolume = ambientVolume;
}

float AsConfiguration::getSoundFxVolume() const
{
	return m_soundFxVolume;
}

void AsConfiguration::setSoundFxVolume(float soundFxVolume)
{
	m_soundFxVolume = soundFxVolume;
}

float AsConfiguration::getMusicVolume() const
{
	return m_musicVolume;
}

void AsConfiguration::setMusicVolume(float musicVolume)
{
	m_musicVolume = musicVolume;
}

bool AsConfiguration::getRumbleEnable() const
{
	return m_rumbleEnable;
}

void AsConfiguration::setRumbleEnable(bool rumbleEnable)
{
	m_rumbleEnable = rumbleEnable;
}

bool AsConfiguration::apply(amalgam::IEnvironment* environment)
{
	PropertyGroup* settings = environment->getSettings();
	T_ASSERT (settings);

	//settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_displayModeWidth);
	//settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_displayModeHeight);
	settings->setProperty< PropertyBoolean >(L"Render.FullScreen", m_fullscreen);
	settings->setProperty< PropertyBoolean >(L"Render.WaitVBlank", m_waitVBlank);
	settings->setProperty< PropertyInteger >(L"Render.MultiSample", m_multiSample);
	settings->setProperty< PropertyFloat >(L"Render.Gamma", m_gamma);
	settings->setProperty< PropertyBoolean >(L"Render.Stereoscopic", m_stereoscopic);
	settings->setProperty< PropertyInteger >(L"Render.TextureQuality", m_textureQuality);
	settings->setProperty< PropertyInteger >(L"World.ShadowQuality", m_shadowQuality);
	settings->setProperty< PropertyFloat >(L"Audio.MasterVolume", m_masterVolume);
	settings->setProperty< PropertyFloat >(L"Audio.AmbientVolume", m_ambientVolume);
	settings->setProperty< PropertyFloat >(L"Audio.SoundFxVolume", m_soundFxVolume);
	settings->setProperty< PropertyFloat >(L"Audio.MusicVolume", m_musicVolume);
	settings->setProperty< PropertyBoolean >(L"Input.Rumble", m_rumbleEnable);

	return environment->reconfigure();
}

	}
}
