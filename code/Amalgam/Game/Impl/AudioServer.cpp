/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Impl/AudioServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Timer/Profiler.h"
#include "Resource/IResourceManager.h"
#include "Sound/ISoundDriver.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundDriverNull.h"
#include "Sound/SoundDriverWriteOut.h"
#include "Sound/SoundFactory.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Player/SoundPlayer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.AudioServer", AudioServer, IAudioServer)

AudioServer::AudioServer()
:	m_autoMute(true)
,	m_soundMuted(false)
,	m_soundMutedVolume(1.0f)
{
}

bool AudioServer::create(const PropertyGroup* settings, const SystemApplication& sysapp)
{
	m_audioType = settings->getProperty< std::wstring >(L"Audio.Type");

	// Create sound driver.
	Ref< sound::ISoundDriver > soundDriver = dynamic_type_cast< sound::ISoundDriver* >(TypeInfo::createInstance(m_audioType));
	if (!soundDriver)
		return false;

	// Create a wrapping "write out" driver if we want to debug audio.
	if (settings->getProperty< bool >(L"Audio.WriteOut", false))
	{
		log::info << L"Creating \"write out\" sound driver wrapper" << Endl;
		soundDriver = new sound::SoundDriverWriteOut(soundDriver);
	}

	// Create sound system.
	m_soundSystem = new sound::SoundSystem(soundDriver);

	sound::SoundSystemCreateDesc sscd;
	sscd.sysapp = sysapp;
#if !defined(_PS3)
	sscd.channels = settings->getProperty< int32_t >(L"Audio.Channels", 16);
	sscd.driverDesc.sampleRate = settings->getProperty< int32_t >(L"Audio.SampleRate", 44100);
	sscd.driverDesc.bitsPerSample = settings->getProperty< int32_t >(L"Audio.BitsPerSample", 16);
	sscd.driverDesc.hwChannels = settings->getProperty< int32_t >(L"Audio.HwChannels", 2);
#	if defined(__IOS__) || defined(__ANDROID__)
	sscd.driverDesc.frameSamples = 1024;
#	else
	sscd.driverDesc.frameSamples = 512;
#	endif
#else
	sscd.channels = 16;
	sscd.driverDesc.sampleRate = 48000;
	sscd.driverDesc.bitsPerSample = 16;
	sscd.driverDesc.hwChannels = 5+1;
	sscd.driverDesc.frameSamples = 4 * 256;
#endif

	if (!m_soundSystem->create(sscd))
	{
		soundDriver = new sound::SoundDriverNull();
		m_soundSystem = new sound::SoundSystem(soundDriver);
		if (!m_soundSystem->create(sscd))
		{
			log::error << L"Audio server failed; unable to create sound system." << Endl;
			m_soundSystem = 0;
			return true;
		}
		log::error << L"Audio server failed; unable to create sound driver, using null driver..." << Endl;
	}

	// Set master volume.
	m_soundSystem->setVolume(settings->getProperty< float >(L"Audio.MasterVolume", 1.0f));
	m_autoMute = settings->getProperty< bool >(L"Audio.AutoMute", true);

	// Set category volumes.
	Ref< const PropertyGroup > volumes = settings->getProperty< PropertyGroup >(L"Audio.Volumes");
	if (volumes)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& cv = volumes->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = cv.begin(); i != cv.end(); ++i)
		{
			const std::wstring& category = i->first;
			float volume = PropertyFloat::get(i->second);
			m_soundSystem->setVolume(
				sound::getParameterHandle(category),
				volume
			);
		}
	}

	// Create surround environment.
	float surroundMaxDistance = settings->getProperty< float >(L"Audio.Surround/MaxDistance", 10.0f);
	float surroundInnerRadius = settings->getProperty< float >(L"Audio.Surround/InnerRadius", 1.0f);
	float surroundFallOffExponent = settings->getProperty< float >(L"Audio.Surround/FallOffExponent", 4.0f);
	m_surroundEnvironment = new sound::SurroundEnvironment(
		surroundMaxDistance,
		surroundInnerRadius,
		surroundFallOffExponent,
		sscd.driverDesc.hwChannels >= 5+1
	);

	// Create high-level sound player.
	m_soundPlayer = new sound::SoundPlayer();
	if (!m_soundPlayer->create(m_soundSystem, m_surroundEnvironment))
	{
		log::error << L"Audio server failed; unable to create sound player, sound muted" << Endl;
		safeDestroy(m_soundSystem);
		m_soundPlayer = 0;
		return true;
	}

	return true;
}

void AudioServer::destroy()
{
	m_surroundEnvironment = 0;
	safeDestroy(m_soundPlayer);
	safeDestroy(m_soundSystem);
}

void AudioServer::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resourceManager->addFactory(new sound::SoundFactory());
}

void AudioServer::update(float dT, bool renderViewActive)
{
	T_PROFILER_SCOPE(L"AudioServer update");

	if (!m_soundSystem || !m_soundPlayer)
		return;

	if (m_autoMute)
	{
		if (!renderViewActive)
		{
			// Should we become muted?
			if (!m_soundMuted)
			{
				T_DEBUG(L"Audio server muted; application inactive");
				m_soundMutedVolume = m_soundSystem->getVolume();
				m_soundMuted = true;
			}

			// Fade down volume until zero.
			float volume = m_soundSystem->getVolume();
			volume = std::max(volume - dT, 0.0f);
			m_soundSystem->setVolume(volume);
		}
		// Fade up volume until "un-muted".
		else if (m_soundMuted)
		{
			float volume = m_soundSystem->getVolume();

			volume += dT;
			if (volume >= m_soundMutedVolume)
			{
				T_DEBUG(L"Audio server un-muted; application active");
				volume = m_soundMutedVolume;
				m_soundMuted = false;
			}

			m_soundSystem->setVolume(volume);
		}
	}

	// Update sound player.
	m_soundPlayer->update(dT);
}

uint32_t AudioServer::getActiveSoundChannels() const
{
	uint32_t activeCount = 0;
	if (m_soundPlayer)
	{
		for (uint32_t i = 0; ; ++i)
		{
			sound::SoundChannel* channel = m_soundSystem->getChannel(i);
			if (!channel)
				break;

			if (channel->isPlaying())
				activeCount++;
		}
	}
	return activeCount;
}

int32_t AudioServer::reconfigure(const PropertyGroup* settings)
{
	if (!m_soundSystem)
		return CrUnaffected;

	// Replace audio driver.
	std::wstring audioType = settings->getProperty< std::wstring >(L"Audio.Type");
	if (audioType != m_audioType)
	{
		Ref< sound::ISoundDriver > soundDriver = dynamic_type_cast< sound::ISoundDriver* >(TypeInfo::createInstance(audioType));
		if (soundDriver && m_soundSystem->reset(soundDriver))
			m_audioType = audioType;
		else
			log::warning << L"Unable to replace sound driver" << Endl;
	}

	// Set master volume.
	m_soundSystem->setVolume(settings->getProperty< float >(L"Audio.MasterVolume", 1.0f));
	m_autoMute = settings->getProperty< bool >(L"Audio.AutoMute", true);

	// Set category volumes.
	Ref< const PropertyGroup > volumes = settings->getProperty< PropertyGroup >(L"Audio.Volumes");
	if (volumes)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& cv = volumes->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = cv.begin(); i != cv.end(); ++i)
		{
			const std::wstring& category = i->first;
			float volume = PropertyFloat::get(i->second);
			m_soundSystem->setVolume(
				sound::getParameterHandle(category),
				volume
			);
		}
	}

	// Configure surround environment distances.
	float surroundMaxDistance = settings->getProperty< float >(L"Audio.Surround/MaxDistance", 10.0f);
	float surroundInnerRadius = settings->getProperty< float >(L"Audio.Surround/InnerRadius", 1.0f);
	m_surroundEnvironment->setMaxDistance(surroundMaxDistance);
	m_surroundEnvironment->setInnerRadius(surroundInnerRadius);

	return CrAccepted;
}

sound::SoundSystem* AudioServer::getSoundSystem()
{
	return m_soundSystem;
}

sound::ISoundPlayer* AudioServer::getSoundPlayer()
{
	return m_soundPlayer;
}

sound::SurroundEnvironment* AudioServer::getSurroundEnvironment()
{
	return m_surroundEnvironment;
}

	}
}
