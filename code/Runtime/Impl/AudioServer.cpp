/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/AudioServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Timer/Profiler.h"
#include "Resource/IResourceManager.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioDriverNull.h"
#include "Sound/AudioDriverWriteOut.h"
#include "Sound/AudioSystem.h"
#include "Sound/IAudioDriver.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Player/SoundPlayer.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.AudioServer", AudioServer, IAudioServer)

bool AudioServer::create(const PropertyGroup* settings, const SystemApplication& sysapp)
{
	m_audioType = settings->getProperty< std::wstring >(L"Audio.Type");

	// Create audio driver.
	Ref< sound::IAudioDriver > audioDriver = dynamic_type_cast< sound::IAudioDriver* >(TypeInfo::createInstance(m_audioType.c_str()));
	if (!audioDriver)
		return false;

	// Create a wrapping "write out" driver if we want to debug audio.
	if (settings->getProperty< bool >(L"Audio.WriteOut", false))
	{
		log::info << L"Creating \"write out\" audio driver wrapper." << Endl;
		audioDriver = new sound::AudioDriverWriteOut(audioDriver);
	}

	// Create audio system.
	m_audioSystem = new sound::AudioSystem(audioDriver);

	sound::AudioSystemCreateDesc ascd;
	ascd.sysapp = sysapp;
	ascd.channels = settings->getProperty< int32_t >(L"Audio.Channels", 16);
	ascd.driverDesc.sampleRate = settings->getProperty< int32_t >(L"Audio.SampleRate", 44100);
	ascd.driverDesc.bitsPerSample = settings->getProperty< int32_t >(L"Audio.BitsPerSample", 16);
	ascd.driverDesc.hwChannels = settings->getProperty< int32_t >(L"Audio.HwChannels", 2);
#	if defined(__IOS__) || defined(__ANDROID__)
	ascd.driverDesc.frameSamples = 1024;
#	else
	ascd.driverDesc.frameSamples = 512;
#	endif

	if (!m_audioSystem->create(ascd))
	{
		audioDriver = new sound::AudioDriverNull();
		m_audioSystem = new sound::AudioSystem(audioDriver);
		if (!m_audioSystem->create(ascd))
		{
			log::error << L"Audio server failed; unable to create audio system." << Endl;
			m_audioSystem = nullptr;
			return true;
		}
		log::error << L"Audio server failed; unable to create audio driver, using null driver..." << Endl;
	}

	// Set master volume.
	m_audioSystem->setVolume(settings->getProperty< float >(L"Audio.MasterVolume", 1.0f));
	m_autoMute = settings->getProperty< bool >(L"Audio.AutoMute", true);

	// Set category volumes.
	Ref< const PropertyGroup > volumes = settings->getProperty< PropertyGroup >(L"Audio.Volumes");
	if (volumes)
	{
		const auto& cv = volumes->getValues();
		for (auto i = cv.begin(); i != cv.end(); ++i)
		{
			const std::wstring& category = i->first;
			const float volume = PropertyFloat::get(i->second);
			m_audioSystem->setVolume(
				sound::getParameterHandle(category),
				volume
			);
		}
	}

	// Create surround environment.
	const float surroundMaxDistance = 25.0f; // settings->getProperty< float >(L"Audio.Surround/MaxDistance", 50.0f);
	const float surroundInnerRadius = 5.0f; // settings->getProperty< float >(L"Audio.Surround/InnerRadius", 5.0f);
	const float surroundFallOffExponent = 4.0f; // settings->getProperty< float >(L"Audio.Surround/FallOffExponent", 4.0f);
	m_surroundEnvironment = new sound::SurroundEnvironment(
		surroundMaxDistance,
		surroundInnerRadius,
		surroundFallOffExponent,
		ascd.driverDesc.hwChannels >= 5+1
	);

	// Create high-level sound player.
	m_soundPlayer = new sound::SoundPlayer();
	if (!m_soundPlayer->create(m_audioSystem, m_surroundEnvironment))
	{
		log::error << L"Audio server failed; unable to create sound player, sound muted" << Endl;
		safeDestroy(m_audioSystem);
		m_soundPlayer = nullptr;
		return true;
	}

	return true;
}

void AudioServer::destroy()
{
	m_surroundEnvironment = nullptr;
	safeDestroy(m_soundPlayer);
	safeDestroy(m_audioSystem);
}

void AudioServer::update(float dT, bool renderViewActive)
{
	T_PROFILER_SCOPE(L"AudioServer update");

	if (!m_audioSystem || !m_soundPlayer)
		return;

	if (m_autoMute)
	{
		if (!renderViewActive)
		{
			// Should we become muted?
			if (!m_soundMuted)
			{
				T_DEBUG(L"Audio server muted; application inactive");
				m_soundMutedVolume = m_audioSystem->getVolume();
				m_soundMuted = true;
			}

			// Fade down volume until zero.
			float volume = m_audioSystem->getVolume();
			volume = std::max(volume - dT, 0.0f);
			m_audioSystem->setVolume(volume);
		}
		// Fade up volume until "un-muted".
		else if (m_soundMuted)
		{
			float volume = m_audioSystem->getVolume();

			volume += dT;
			if (volume >= m_soundMutedVolume)
			{
				T_DEBUG(L"Audio server un-muted; application active");
				volume = m_soundMutedVolume;
				m_soundMuted = false;
			}

			m_audioSystem->setVolume(volume);
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
			sound::AudioChannel* channel = m_audioSystem->getChannel(i);
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
	if (!m_audioSystem)
		return CrUnaffected;

	// Replace audio driver.
	const std::wstring audioType = settings->getProperty< std::wstring >(L"Audio.Type");
	if (audioType != m_audioType)
	{
		Ref< sound::IAudioDriver > soundDriver = dynamic_type_cast< sound::IAudioDriver* >(TypeInfo::createInstance(audioType.c_str()));
		if (soundDriver && m_audioSystem->reset(soundDriver))
			m_audioType = audioType;
		else
			log::warning << L"Unable to replace sound driver" << Endl;
	}

	// Set master volume.
	m_audioSystem->setVolume(settings->getProperty< float >(L"Audio.MasterVolume", 1.0f));
	m_autoMute = settings->getProperty< bool >(L"Audio.AutoMute", true);

	// Set category volumes.
	Ref< const PropertyGroup > volumes = settings->getProperty< PropertyGroup >(L"Audio.Volumes");
	if (volumes)
	{
		const auto& cv = volumes->getValues();
		for (auto i = cv.begin(); i != cv.end(); ++i)
		{
			const std::wstring& category = i->first;
			const float volume = PropertyFloat::get(i->second);
			m_audioSystem->setVolume(
				sound::getParameterHandle(category),
				volume
			);
		}
	}

	// Configure surround environment distances.
	//const float surroundMaxDistance = settings->getProperty< float >(L"Audio.Surround/MaxDistance", 10.0f);
	//const float surroundInnerRadius = settings->getProperty< float >(L"Audio.Surround/InnerRadius", 1.0f);
	//m_surroundEnvironment->setMaxDistance(surroundMaxDistance);
	//m_surroundEnvironment->setInnerRadius(surroundInnerRadius);
	return CrAccepted;
}

sound::AudioSystem* AudioServer::getAudioSystem()
{
	return m_audioSystem;
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
