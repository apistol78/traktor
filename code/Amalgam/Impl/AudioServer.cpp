#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/AudioServer.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Resource/IResourceManager.h"
#include "Sound/ISoundDriver.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundFactory.h"
#include "Sound/Filters/SurroundEnvironment.h"

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

bool AudioServer::create(const Settings* settings)
{
	if (!settings->getProperty< PropertyBoolean >(L"Audio.Enabled", true))
	{
		log::info << L"Audio disabled; audio server initialization skipped" << Endl;
		return true;
	}

	std::wstring audioType = settings->getProperty< PropertyString >(L"Audio.Type");

	Ref< sound::ISoundDriver > soundDriver = loadAndInstantiate< sound::ISoundDriver >(audioType);
	if (!soundDriver)
		return false;

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

	float surroundMaxDistance = settings->getProperty< PropertyFloat >(L"Audio.Surround/MaxDistance", 10.0f);
	float surroundInnerRadius = settings->getProperty< PropertyFloat >(L"Audio.Surround/InnerRadius", 1.0f);
	m_surroundEnvironment = new sound::SurroundEnvironment(
		surroundMaxDistance,
		surroundInnerRadius,
		sscd.driverDesc.hwChannels >= 5+1
	);

	m_soundSystem->setVolume(settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f));
	return true;
}

void AudioServer::destroy()
{
	m_surroundEnvironment = 0;
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
	if (!m_soundSystem)
		return;

	if (!renderViewActive)
	{
		// Should we become muted?
		if (!m_soundMuted)
		{
			log::debug << L"Audio server muted; application inactive" << Endl;
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
			log::debug << L"Audio server un-muted; application active" << Endl;
			volume = m_soundMutedVolume;
			m_soundMuted = false;
		}

		m_soundSystem->setVolume(volume);
	}
}

int32_t AudioServer::reconfigure(const Settings* settings)
{
	if (!m_soundSystem)
		return CrUnaffected;

	m_soundSystem->setVolume(settings->getProperty< PropertyFloat >(L"Audio.MasterVolume", 1.0f));

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

sound::SurroundEnvironment* AudioServer::getSurroundEnvironment()
{
	return m_surroundEnvironment;
}

	}
}
