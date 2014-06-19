#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/Action/AsConfiguration.h"
#include "Amalgam/Engine/Action/AsDisplayMode.h"
#include "Amalgam/Engine/Action/AsSoundDriver.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
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
,	m_particleQuality(QtMedium)
,	m_terrainQuality(QtMedium)
,	m_oceanQuality(QtMedium)
,	m_postProcessQuality(QtHigh)
,	m_mouseSensitivity(0.5f)
,	m_rumbleEnable(true)
,	m_masterVolume(1.0f)
,	m_autoMute(true)
{
}

Ref< AsConfiguration > AsConfiguration::getCurrent(amalgam::IEnvironment* environment)
{
	const PropertyGroup* settings = environment->getSettings();
	T_ASSERT (settings);

	Ref< AsConfiguration > current = new AsConfiguration();
	current->m_settings = settings;

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
	current->m_particleQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.ParticleQuality", QtMedium);
	current->m_terrainQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.TerrainQuality", QtMedium);
	current->m_oceanQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.OceanQuality", QtMedium);
	current->m_postProcessQuality = (Quality)settings->getProperty< PropertyInteger >(L"World.PostProcessQuality", QtHigh);
	current->m_mouseSensitivity = settings->getProperty< PropertyFloat >(L"Input.MouseSensitivity", 0.5f);
	current->m_rumbleEnable = settings->getProperty< PropertyBoolean >(L"Input.Rumble", true);
	current->m_soundDriver = settings->getProperty< PropertyString >(L"Audio.Type");
	current->m_masterVolume = settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f);
	current->m_autoMute = settings->getProperty< PropertyBoolean >(L"Audio.AutoMute", true);

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

AsConfiguration::Quality AsConfiguration::getParticleQuality() const
{
	return m_particleQuality;
}

void AsConfiguration::setParticleQuality(Quality particleQuality)
{
	m_particleQuality = particleQuality;
}

AsConfiguration::Quality AsConfiguration::getTerrainQuality() const
{
	return m_terrainQuality;
}

void AsConfiguration::setTerrainQuality(Quality terrainQuality)
{
	m_terrainQuality = terrainQuality;
}

AsConfiguration::Quality AsConfiguration::getOceanQuality() const
{
	return m_oceanQuality;
}

void AsConfiguration::setOceanQuality(Quality oceanQuality)
{
	m_oceanQuality = oceanQuality;
}

AsConfiguration::Quality AsConfiguration::getPostProcessQuality() const
{
	return m_postProcessQuality;
}

void AsConfiguration::setPostProcessQuality(Quality postProcessQuality)
{
	m_postProcessQuality = postProcessQuality;
}

float AsConfiguration::getMouseSensitivity() const
{
	return m_mouseSensitivity;
}

void AsConfiguration::setMouseSensitivity(float mouseSensitivity)
{
	m_mouseSensitivity = mouseSensitivity;
}

bool AsConfiguration::getRumbleEnable() const
{
	return m_rumbleEnable;
}

void AsConfiguration::setRumbleEnable(bool rumbleEnable)
{
	m_rumbleEnable = rumbleEnable;
}

Ref< AsSoundDriver > AsConfiguration::getSoundDriver() const
{
	return new AsSoundDriver(TypeInfo::find(m_soundDriver));
}

void AsConfiguration::setSoundDriver(const AsSoundDriver* soundDriver)
{
	if (soundDriver)
		m_soundDriver = soundDriver->getName();
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

bool AsConfiguration::getAutoMute() const
{
	return m_autoMute;
}

void AsConfiguration::setAutoMute(bool autoMute)
{
	m_autoMute = autoMute;
}

bool AsConfiguration::getBoolean(const std::wstring& name) const
{
	std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = m_user.find(name);
	if (i != m_user.end())
		return PropertyBoolean::get(i->second);

	if (m_settings)
		return m_settings->getProperty< PropertyBoolean >(name);
	else
		return false;
}

void AsConfiguration::setBoolean(const std::wstring& name, bool value)
{
	m_user[name] = new PropertyBoolean(value);
}

int32_t AsConfiguration::getInteger(const std::wstring& name) const
{
	std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = m_user.find(name);
	if (i != m_user.end())
		return PropertyInteger::get(i->second);

	if (m_settings)
		return m_settings->getProperty< PropertyInteger >(name);
	else
		return false;
}

void AsConfiguration::setInteger(const std::wstring& name, int32_t value)
{
	m_user[name] = new PropertyInteger(value);
}

float AsConfiguration::getFloat(const std::wstring& name) const
{
	std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = m_user.find(name);
	if (i != m_user.end())
		return PropertyFloat::get(i->second);

	if (m_settings)
		return m_settings->getProperty< PropertyFloat >(name);
	else
		return false;
}

void AsConfiguration::setFloat(const std::wstring& name, float value)
{
	m_user[name] = new PropertyFloat(value);
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
	settings->setProperty< PropertyInteger >(L"World.ParticleQuality", m_particleQuality);
	settings->setProperty< PropertyInteger >(L"World.TerrainQuality", m_terrainQuality);
	settings->setProperty< PropertyInteger >(L"World.OceanQuality", m_oceanQuality);
	settings->setProperty< PropertyInteger >(L"World.PostProcessQuality", m_postProcessQuality);
	settings->setProperty< PropertyFloat >(L"Input.MouseSensitivity", m_mouseSensitivity);
	settings->setProperty< PropertyBoolean >(L"Input.Rumble", m_rumbleEnable);
	settings->setProperty< PropertyString >(L"Audio.Type", m_soundDriver);
	settings->setProperty< PropertyFloat >(L"Audio.MasterVolume", m_masterVolume);
	settings->setProperty< PropertyBoolean >(L"Audio.AutoMute", m_autoMute);

	for (std::map< std::wstring, float >::const_iterator i = m_volumes.begin(); i != m_volumes.end(); ++i)
		settings->setProperty< PropertyFloat >(L"Audio.Volumes/" + i->first, i->second);

	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = m_user.begin(); i != m_user.end(); ++i)
		settings->setProperty(i->first, i->second);

	return environment->reconfigure();
}

	}
}
