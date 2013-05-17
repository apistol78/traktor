#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/AudioServer.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Resource/IResourceManager.h"
#include "Sound/ISoundDriver.h"
#include "Sound/SoundChannel.h"
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
:	m_soundMuted(false)
,	m_soundMutedVolume(1.0f)
{
}

bool AudioServer::create(const PropertyGroup* settings)
{
	std::wstring audioType = settings->getProperty< PropertyString >(L"Audio.Type");

	// Create sound driver.
	Ref< sound::ISoundDriver > soundDriver = loadAndInstantiate< sound::ISoundDriver >(audioType);
	if (!soundDriver)
		return false;

	// Create a wrapping "write out" driver if we want to debug audio.
	if (settings->getProperty< PropertyBoolean >(L"Audio.WriteOut", false))
	{
		log::info << L"Creating \"write out\" sound driver wrapper" << Endl;
		soundDriver = new sound::SoundDriverWriteOut(soundDriver);
	}

	// Create sound system.
	m_soundSystem = new sound::SoundSystem(soundDriver);

	sound::SoundSystemCreateDesc sscd;
#if !defined(_PS3)
	sscd.channels = settings->getProperty< PropertyInteger >(L"Audio.Channels", 16);
	sscd.driverDesc.sampleRate = settings->getProperty< PropertyInteger >(L"Audio.SampleRate", 44100);
	sscd.driverDesc.bitsPerSample = settings->getProperty< PropertyInteger >(L"Audio.BitsPerSample", 16);
	sscd.driverDesc.hwChannels = settings->getProperty< PropertyInteger >(L"Audio.HwChannels", 2);
	sscd.driverDesc.frameSamples = 1024;
	sscd.driverDesc.mixerFrames = 3;
#else
	sscd.channels = 16;
	sscd.driverDesc.sampleRate = 48000;
	sscd.driverDesc.bitsPerSample = 16;
	sscd.driverDesc.hwChannels = 5+1;
	sscd.driverDesc.frameSamples = 4 * 256;
	sscd.driverDesc.mixerFrames = 5;
#endif

	if (!m_soundSystem->create(sscd))
	{
		log::error << L"Audio server failed; unable to create sound system, sound muted" << Endl;
		m_soundSystem = 0;
		return true;
	}

	// Set master volume.
	m_soundSystem->setVolume(settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f));

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
	float surroundMaxDistance = settings->getProperty< PropertyFloat >(L"Audio.Surround/MaxDistance", 10.0f);
	float surroundInnerRadius = settings->getProperty< PropertyFloat >(L"Audio.Surround/InnerRadius", 1.0f);
	m_surroundEnvironment = new sound::SurroundEnvironment(
		surroundMaxDistance,
		surroundInnerRadius,
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
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new sound::SoundFactory(database));
}

void AudioServer::update(float dT, bool renderViewActive)
{
	if (!m_soundSystem || !m_soundPlayer)
		return;

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

	// Set master volume.
	m_soundSystem->setVolume(settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f));

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
	float surroundMaxDistance = settings->getProperty< PropertyFloat >(L"Audio.Surround/MaxDistance", 10.0f);
	float surroundInnerRadius = settings->getProperty< PropertyFloat >(L"Audio.Surround/InnerRadius", 1.0f);
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
