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
#include "Sound/ISoundDriver.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundMixer.h"
#include "Sound/SoundSystem.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

inline void clearSamples(float* samples, int32_t samplesCount)
{
	T_ASSERT ((samplesCount & 3) == 0);

	const static Vector4 c_zero4 = Vector4::zero();
	int32_t i = 0;

	for (i = 0; i < samplesCount - 16; i += 16)
	{
		c_zero4.storeAligned(&samples[i]);
		c_zero4.storeAligned(&samples[i + 4]);
		c_zero4.storeAligned(&samples[i + 8]);
		c_zero4.storeAligned(&samples[i + 12]);
	}

	for (; i < samplesCount; i += 4)
		c_zero4.storeAligned(&samples[i]);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundSystem", SoundSystem, Object)

SoundSystem::SoundSystem(ISoundDriver* driver)
:	m_driver(driver)
,	m_suspended(false)
,	m_volume(1.0f)
,	m_threadMixer(0)
,	m_threadSubmit(0)
,	m_samplesData(0)
,	m_samplesBlockHead(0)
,	m_samplesBlockTail(0)
,	m_time(0.0)
,	m_mixerThreadTime(0.0)
,	m_submitThreadTime(0.0)
{
}

bool SoundSystem::create(const SoundSystemCreateDesc& desc)
{
	T_ASSERT (m_driver);

	m_desc = desc;
	m_desc.driverDesc.frameSamples &= ~3U;

	// Create driver.
	if (!m_driver->create(m_desc.nativeHandle, m_desc.driverDesc, m_mixer))
		return false;

	// If driver didn't create an alternative sound mixer we create
	// a default mixer.
	if (!m_mixer)
		m_mixer = new SoundMixer();

	// Allocate samples.
	uint32_t samplesBlockCount = m_desc.driverDesc.mixerFrames * 2;
	uint32_t samplesPerBlock = m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels;

	m_samplesData = static_cast< float* >(Alloc::acquireAlign(
		samplesPerBlock * samplesBlockCount * sizeof(float),
		16,
		T_FILE_LINE
	));
	if (!m_samplesData)
		return false;

	for (uint32_t i = 0; i < samplesBlockCount; ++i)
		m_samplesBlocks.push_back(&m_samplesData[i * samplesPerBlock]);

	// Create mixer and submission threads.
	m_threadMixer = ThreadManager::getInstance().create(makeFunctor(this, &SoundSystem::threadMixer), L"Sound mixer", 1);
	if (!m_threadMixer)
	{
		m_driver->destroy();
		return false;
	}

	m_threadSubmit = ThreadManager::getInstance().create(makeFunctor(this, &SoundSystem::threadSubmit), L"Sound submit", 2);
	if (!m_threadSubmit)
	{
		m_driver->destroy();
		return false;
	}

	// Create virtual channels.
	m_channels.resize(desc.channels);
	for (uint32_t i = 0; i < desc.channels; ++i)
	{
		m_channels[i] = new SoundChannel(
			i,
			desc.driverDesc.sampleRate,
			desc.driverDesc.frameSamples
		);
	}

	m_requestBlocks.resize(desc.channels);
	m_requestBlockMeta.resize(desc.channels);

	// Set play parameters.
	m_time = 0.0;
	m_duck[0].resize(desc.channels, 1.0f);
	m_duck[1].resize(desc.channels, 1.0f);

	// Start threads.
	m_threadMixer->start(Thread::Above);
	m_threadSubmit->start(Thread::Highest);

	return true;
}

void SoundSystem::destroy()
{
	if (m_threadSubmit)
	{
		m_threadSubmit->stop();
		ThreadManager::getInstance().destroy(m_threadSubmit);
		m_threadSubmit = 0;
	}

	if (m_threadMixer)
	{
		m_threadMixer->stop();
		ThreadManager::getInstance().destroy(m_threadMixer);
		m_threadMixer = 0;
	}

	m_mixer = 0;
	safeDestroy(m_driver);

	if (m_samplesData)
	{
		Alloc::freeAlign(m_samplesData);
		m_samplesData = 0;
	}
}

bool SoundSystem::reset(ISoundDriver* driver)
{
	// Tear down current driver and threads.
	suspend();
	m_driver = 0;

	// Create new driver and mixer.
	if (!driver || !driver->create(m_desc.nativeHandle, m_desc.driverDesc, m_mixer))
		return false;

	m_driver = driver;

	// If driver didn't create an alternative sound mixer we create the default mixer.
	if (!m_mixer)
		m_mixer = new SoundMixer();

	// Restart mixer and submission threads.
	resume();
	return true;
}

void SoundSystem::suspend()
{
	// Prevent multiple suspends.
	if (m_suspended)
		return;

	if (m_threadSubmit)
	{
		m_threadSubmit->stop();
		ThreadManager::getInstance().destroy(m_threadSubmit);
		m_threadSubmit = 0;
	}

	if (m_threadMixer)
	{
		m_threadMixer->stop();
		ThreadManager::getInstance().destroy(m_threadMixer);
		m_threadMixer = 0;
	}

	// Destroy driver; but keep pointer to driver, as we will re-create it.
	if (m_driver)
		m_driver->destroy();

	m_mixer = 0;
	m_suspended = true;
}

void SoundSystem::resume()
{
	// Prevent multiple resumes.
	if (!m_suspended)
		return;

	T_ASSERT (m_threadMixer == 0);
	T_ASSERT (m_threadSubmit == 0);

	if (!m_driver)
		return;

	// Re-instanciate driver.
	m_driver = checked_type_cast< ISoundDriver* >(type_of(m_driver).createInstance());
	if (!m_driver)
		return;

	// Create driver.
	if (!m_driver->create(m_desc.nativeHandle, m_desc.driverDesc, m_mixer))
		return;

	// If driver didn't create an alternative sound mixer we create
	// a default mixer.
	if (!m_mixer)
		m_mixer = new SoundMixer();

	// Create threads.
	if (!m_threadMixer)
	{
		m_threadMixer = ThreadManager::getInstance().create(makeFunctor(this, &SoundSystem::threadMixer), L"Sound mixer", 1);
		if (!m_threadMixer)
			return;
	}

	if (!m_threadSubmit)
	{
		m_threadSubmit = ThreadManager::getInstance().create(makeFunctor(this, &SoundSystem::threadSubmit), L"Sound submit", 2);
		if (!m_threadSubmit)
			return;
	}

	// Start threads.
	m_threadMixer->start(Thread::Above);
	m_threadSubmit->start(Thread::Highest);

	m_suspended = false;
}

void SoundSystem::setVolume(float volume)
{
	m_volume = volume;
}

float SoundSystem::getVolume() const
{
	return m_volume;
}

void SoundSystem::setVolume(handle_t category, float volume)
{
	m_categoryVolumes[category] = volume;
}

float SoundSystem::getVolume(handle_t category) const
{
	SmallMap< handle_t, float >::const_iterator i = m_categoryVolumes.find(category);
	return i != m_categoryVolumes.end() ? i->second : 1.0f;
}

void SoundSystem::setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount])
{
	std::memcpy(m_desc.cm, cm, sizeof(float) * SbcMaxChannelCount * SbcMaxChannelCount);
}

Ref< SoundChannel > SoundSystem::getChannel(uint32_t channelId)
{
	if (channelId < m_channels.size())
		return m_channels[channelId];
	else
		return 0;
}

double SoundSystem::getTime() const
{
	return m_time;
}

void SoundSystem::getThreadPerformances(double& outMixerTime, double& outSubmitTime) const
{
	outMixerTime = m_mixerThreadTime;
	outSubmitTime = m_submitThreadTime;
}

void SoundSystem::threadMixer()
{
	SoundBlock frameBlock;
	Timer timerMixer;

	timerMixer.start();
	while (!m_threadMixer->stopped())
	{
		// Wait until submission queue is below threshold.
		m_submitQueueLock.wait();
		while (m_submitQueue.size() >= m_desc.driverDesc.mixerFrames && !m_threadMixer->stopped())
		{
			m_submitQueueLock.release();
			m_submitConsumedEvent.wait(100);
			m_submitQueueLock.wait();
		}
		m_submitQueueLock.release();

		if (m_threadMixer->stopped())
			break;

		double startTime = timerMixer.getElapsedTime();
		double deltaTime = timerMixer.getDeltaTime();

		// Read blocks from channels.
		uint32_t channelsCount = uint32_t(m_channels.size());
		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			m_requestBlocks[i].samplesCount = m_desc.driverDesc.frameSamples;
			m_requestBlocks[i].maxChannel = 0;

			m_requestBlockMeta[i].category = 0;
			m_requestBlockMeta[i].presence = 0.0f;
			m_requestBlockMeta[i].presenceRate = 0.0f;

			m_channels[i]->getBlock(m_mixer, m_time, m_requestBlocks[i], m_requestBlockMeta[i]);
		}

		// Allocate new frame block.
		float* samples = m_samplesBlocks[m_samplesBlockHead];
		m_samplesBlocks[m_samplesBlockHead] = 0;
		m_samplesBlockHead = (m_samplesBlockHead + 1) % m_samplesBlocks.size();
		T_ASSERT (samples);

		// Prepare new frame block.
		clearSamples(samples, m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels);
		for (uint32_t i = 0; i < m_desc.driverDesc.hwChannels; ++i)
			frameBlock.samples[i] = samples + m_desc.driverDesc.frameSamples * i;
		frameBlock.samplesCount = m_desc.driverDesc.frameSamples;
		frameBlock.sampleRate = m_desc.driverDesc.sampleRate;
		frameBlock.maxChannel = m_desc.driverDesc.hwChannels;

		// Get which channel has the highest presence.
		float presence[64];
		float maxPresence = 1.0f;

		for (int32_t i = 0; i < int32_t(channelsCount); ++i)
		{
			if (m_requestBlockMeta[i].presence > 0.0f)
			{
				presence[i] = 1.0f + m_requestBlockMeta[i].presence;
				maxPresence = max(maxPresence, presence[i]);
			}
			else
				presence[i] = 1.0f;
		}

		for (uint32_t i = 0; i < m_duck[0].size(); ++i)
		{
			if (m_requestBlockMeta[i].presence > 0.0f)
			{
				m_duck[0][i] = presence[i] / maxPresence;
				m_duck[0][i] *= m_duck[0][i];

				if (m_duck[1][i] > m_duck[0][i])
					m_duck[1][i] = m_duck[0][i];
				else
				{
					float presenceRate = m_requestBlockMeta[i].presenceRate;
					m_duck[1][i] = min(m_duck[1][i] + float(deltaTime * presenceRate), 1.0f);
				}
			}
		}

		m_mixer->synchronize();

		// Final combine channels into hardware channels using "combine matrix".
		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			if (!m_requestBlocks[i].maxChannel)
				continue;

			T_ASSERT (m_requestBlocks[i].sampleRate == m_desc.driverDesc.sampleRate);
			T_ASSERT (m_requestBlocks[i].samplesCount == m_desc.driverDesc.frameSamples);

			float categoryVolume = getVolume(m_requestBlockMeta[i].category);
			float duck = m_volume * categoryVolume * (m_duck[1][i] * 0.5f + 0.5f);

			for (uint32_t k = 0; k < m_requestBlocks[i].maxChannel; ++k)
			{
				if (!m_requestBlocks[i].samples[k])
					continue;

				for (uint32_t j = 0; j < m_desc.driverDesc.hwChannels; ++j)
				{
					float strength = m_desc.cm[j][k] * duck;
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

		m_submitQueueLock.wait();
		m_submitQueue.push_back(frameBlock);
		m_submitQueueLock.release();
		m_submitQueueEvent.broadcast();

		double endTime = timerMixer.getElapsedTime();

		m_mixerThreadTime = (endTime - startTime) * 0.1f + m_mixerThreadTime * 0.9f;
	}
}

void SoundSystem::threadSubmit()
{
	Timer timerSubmit;

	timerSubmit.start();
	while (!m_threadSubmit->stopped())
	{
		m_submitQueueLock.wait();
		if (m_submitQueue.empty())
		{
			log::warning << L"Sound - submit thread starved, waiting for mixer to catch up" << Endl;
			while (m_submitQueue.size() < m_desc.driverDesc.mixerFrames && !m_threadSubmit->stopped())
			{
				m_submitQueueLock.release();
				m_submitQueueEvent.wait(100);
				m_submitQueueLock.wait();
			}
			if (m_threadSubmit->stopped())
				break;
		}

		// Submit block to driver.
		double startTime = timerSubmit.getElapsedTime();
		m_driver->submit(m_submitQueue.front());

		// Move block back into heap.
		T_ASSERT (m_samplesBlocks[m_samplesBlockTail] == 0);
		m_samplesBlocks[m_samplesBlockTail] = m_submitQueue.front().samples[0];
		m_samplesBlockTail = (m_samplesBlockTail + 1) % m_samplesBlocks.size();

		m_submitQueue.pop_front();
		m_submitQueueLock.release();

		m_submitConsumedEvent.broadcast();

		double endTime = timerSubmit.getElapsedTime();

		// Wait until driver is ready to play another block.
		m_driver->wait();

		m_submitThreadTime = (endTime - startTime) * 0.1f + m_submitThreadTime * 0.9f;
	}
}

	}
}
