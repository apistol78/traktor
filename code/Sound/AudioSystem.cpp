/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioMixer.h"
#include "Sound/AudioSystem.h"
#include "Sound/IAudioDriver.h"
#include "Sound/IAudioMixer.h"
#include "Sound/Sound.h"

#if defined(T_SOUND_USE_AVX_MIXER)
#	include "Sound/Avx/AudioMixerAvx.h"
#endif

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.AudioSystem", AudioSystem, Object)

AudioSystem::AudioSystem(IAudioDriver* driver)
:	m_driver(driver)
,	m_suspended(false)
,	m_volume(1.0f)
,	m_threadMixer(0)
,	m_samplesData(0)
,	m_time(0.0)
,	m_mixerThreadTime(0.0)
{
}

bool AudioSystem::create(const AudioSystemCreateDesc& desc)
{
	T_ASSERT(m_driver);

	m_desc = desc;
	m_desc.driverDesc.frameSamples &= ~3U;

	// Create driver.
	if (!m_driver->create(m_desc.sysapp, m_desc.driverDesc, m_mixer))
		return false;

	// If driver didn't create an alternative sound mixer we create
	// a default mixer.
	if (!m_mixer)
	{
#if defined(T_SOUND_USE_AVX_MIXER)
		if (AudioMixerAvx::supported())
			m_mixer = new AudioMixerAvx();
#endif
		if (!m_mixer)
			m_mixer = new AudioMixer();
	}

	// Allocate samples.
	const uint32_t samplesBlockCount = m_samplesBlocks.capacity();
	uint32_t samplesPerBlock = m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels;

	m_samplesData = static_cast< float* >(Alloc::acquireAlign(
		samplesPerBlock * samplesBlockCount * sizeof(float),
		16,
		T_FILE_LINE
	));
	if (!m_samplesData)
		return false;

	std::memset(m_samplesData, 0, samplesPerBlock * samplesBlockCount * sizeof(float));
	for (uint32_t i = 0; i < samplesBlockCount; ++i)
		m_samplesBlocks.push_back(&m_samplesData[i * samplesPerBlock]);

	// Create mixer and submission threads.
	m_threadMixer = ThreadManager::getInstance().create([=, this](){ threadMixer(); }, L"Sound mixer", 1);
	if (!m_threadMixer)
	{
		m_driver->destroy();
		return false;
	}

	// Create virtual channels.
	m_channels.resize(desc.channels);
	for (uint32_t i = 0; i < desc.channels; ++i)
	{
		m_channels[i] = new AudioChannel(
			i,
			desc.driverDesc.sampleRate,
			desc.driverDesc.frameSamples
		);
	}

	// Set play parameters.
	m_requestBlocks.resize(desc.channels);
	m_time = 0.0;

	// Start thread.
	m_threadMixer->start(Thread::Above);
	return true;
}

void AudioSystem::destroy()
{
	// Release all channels to ensure submission thread no longer tries to request blocks from channels.
	{
		m_channelsLock.wait();
		m_channels.clear();
		m_channelsLock.release();
	}

	// Terminate mixer thread.
	if (m_threadMixer)
	{
		m_threadMixer->stop();
		ThreadManager::getInstance().destroy(m_threadMixer);
		m_threadMixer = nullptr;
	}

	// Free mixer and memory resources.
	m_mixer = nullptr;
	safeDestroy(m_driver);

	if (m_samplesData)
	{
		Alloc::freeAlign(m_samplesData);
		m_samplesData = nullptr;
	}
}

bool AudioSystem::reset(IAudioDriver* driver)
{
	// Tear down current driver and threads.
	suspend();
	m_driver = nullptr;

	// Create new driver and mixer.
	if (!driver || !driver->create(m_desc.sysapp, m_desc.driverDesc, m_mixer))
		return false;

	m_driver = driver;

	// If driver didn't create an alternative sound mixer we create the default mixer.
	if (!m_mixer)
		m_mixer = new AudioMixer();

	// Restart mixer and submission threads.
	resume();
	return true;
}

void AudioSystem::suspend()
{
	// Prevent multiple suspends.
	if (m_suspended)
		return;

	if (m_threadMixer)
	{
		m_threadMixer->stop();
		ThreadManager::getInstance().destroy(m_threadMixer);
		m_threadMixer = nullptr;
	}

	// Destroy driver; but keep pointer to driver, as we will re-create it.
	if (m_driver)
		m_driver->destroy();

	m_mixer = nullptr;
	m_suspended = true;
}

void AudioSystem::resume()
{
	// Prevent multiple resumes.
	if (!m_suspended)
		return;

	T_ASSERT(m_threadMixer == nullptr);

	if (!m_driver)
		return;

	// Re-instanciate driver.
	m_driver = checked_type_cast< IAudioDriver* >(type_of(m_driver).createInstance());
	if (!m_driver)
		return;

	// Create driver.
	if (!m_driver->create(m_desc.sysapp, m_desc.driverDesc, m_mixer))
		return;

	// If driver didn't create an alternative sound mixer we create
	// a default mixer.
	if (!m_mixer)
		m_mixer = new AudioMixer();

	// Create threads.
	if (!m_threadMixer)
	{
		m_threadMixer = ThreadManager::getInstance().create([=, this](){ threadMixer(); }, L"Sound mixer", 1);
		if (!m_threadMixer)
			return;
	}

	// Start threads.
	m_threadMixer->start(Thread::Above);
	m_suspended = false;
}

void AudioSystem::setVolume(float volume)
{
	m_volume = volume;
}

float AudioSystem::getVolume() const
{
	return m_volume;
}

void AudioSystem::setVolume(handle_t category, float volume)
{
	m_categoryVolumes[category] = volume;
}

float AudioSystem::getVolume(handle_t category) const
{
	SmallMap< handle_t, float >::const_iterator i = m_categoryVolumes.find(category);
	return i != m_categoryVolumes.end() ? i->second : 1.0f;
}

void AudioSystem::setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount])
{
	std::memcpy(m_desc.cm, cm, sizeof(float) * SbcMaxChannelCount * SbcMaxChannelCount);
}

AudioChannel* AudioSystem::getChannel(uint32_t channelId)
{
	if (channelId < m_channels.size())
		return m_channels[channelId];
	else
		return nullptr;
}

double AudioSystem::getTime() const
{
	return m_time;
}

void AudioSystem::getThreadPerformances(double& outMixerTime) const
{
	outMixerTime = m_mixerThreadTime;
}

void AudioSystem::threadMixer()
{
	AudioBlock frameBlock;
	Timer timerMixer;
	uint32_t channelsCount;

	timerMixer.reset();
	while (!m_threadMixer->stopped())
	{
		double startTime = timerMixer.getElapsedTime();
		double deltaTime = timerMixer.getDeltaTime();

		// Read blocks from channels.
		m_channelsLock.wait();
		{
			channelsCount = uint32_t(m_channels.size());
			for (uint32_t i = 0; i < channelsCount; ++i)
			{
				m_requestBlocks[i].samplesCount = m_desc.driverDesc.frameSamples;
				m_requestBlocks[i].maxChannel = 0;
				m_requestBlocks[i].category = 0;
				m_channels[i]->getBlock(m_mixer, m_requestBlocks[i]);
			}
		}
		m_channelsLock.release();

		// Allocate new frame block.
		float* samples = m_samplesBlocks.front();
		m_samplesBlocks.pop_front();

		// Prepare new frame block.
		m_mixer->mute(samples, m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels);
		for (uint32_t i = 0; i < m_desc.driverDesc.hwChannels; ++i)
			frameBlock.samples[i] = samples + m_desc.driverDesc.frameSamples * i;
		frameBlock.samplesCount = m_desc.driverDesc.frameSamples;
		frameBlock.sampleRate = m_desc.driverDesc.sampleRate;
		frameBlock.maxChannel = m_desc.driverDesc.hwChannels;

		m_mixer->synchronize();

		// Final combine channels into hardware channels using "combine matrix".
		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			if (!m_requestBlocks[i].maxChannel)
				continue;

			T_ASSERT(m_requestBlocks[i].sampleRate == m_desc.driverDesc.sampleRate);
			T_ASSERT(m_requestBlocks[i].samplesCount == m_desc.driverDesc.frameSamples);

			const float categoryVolume = getVolume(m_requestBlocks[i].category);
			const float finalVolume = m_volume * categoryVolume;

			for (uint32_t k = 0; k < m_requestBlocks[i].maxChannel; ++k)
			{
				if (!m_requestBlocks[i].samples[k])
					continue;

				for (uint32_t j = 0; j < m_desc.driverDesc.hwChannels; ++j)
				{
					const float strength = m_desc.cm[j][k] * finalVolume;
					if (abs(strength) >= FUZZY_EPSILON)
					{
						m_mixer->addMulConst(
							frameBlock.samples[j],
							m_requestBlocks[i].samples[k],
							m_requestBlocks[i].samplesCount,
							strength
						);
					}
				}

				m_mixer->synchronize();
			}
		}

		m_time += double(m_desc.driverDesc.frameSamples) / m_desc.driverDesc.sampleRate;

		if (m_threadMixer->stopped())
			break;

		// Submit frame block to driver.
		m_driver->wait();
		m_driver->submit(frameBlock);

		// Move block back into heap.
		m_samplesBlocks.push_back(frameBlock.samples[0]);

		double endTime = timerMixer.getElapsedTime();
		m_mixerThreadTime = (endTime - startTime) * 0.1f + m_mixerThreadTime * 0.9f;
	}
}

}
