#include "Sound/SoundSystem.h"
#include "Sound/SoundDriver.h"
#include "Sound/SoundChannel.h"
#include "Sound/Sound.h"
#include "Core/Heap/HeapNew.h"
#include "Core/Heap/Alloc.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_allocateBlocks = 4;
const uint32_t c_mixerFramesAhead = 2;	//< Number of frames the mixer thread should be in front of submission thread.

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundSystem", SoundSystem, Object)

SoundSystem::SoundSystem(SoundDriver* driver)
:	m_driver(driver)
,	m_threadMixer(0)
,	m_threadSubmit(0)
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
	if (!m_driver->create(m_desc.driverDesc))
		return false;

	// Allocate samples.
	for (uint32_t i = 0; i < c_allocateBlocks; ++i)
	{
		float* block = static_cast< float* >(allocAlign(
			m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels * sizeof(float),
			16
		));
		if (!block)
			return false;

		m_samplesBlocks.push_back(block);
	}

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
		m_channels[i] = gc_new< SoundChannel >(
			desc.driverDesc.sampleRate,
			desc.driverDesc.frameSamples
		);
	}

	// Reset global playback time.
	m_time = 0.0;

	// Start threads.
	m_threadMixer->start();
	m_threadSubmit->start(Thread::Above);

	return true;
}

void SoundSystem::destroy()
{
	T_ASSERT (m_driver);

	if (m_threadSubmit)
	{
		m_threadSubmit->stop();
		ThreadManager::getInstance().destroy(m_threadSubmit);
	}

	if (m_threadMixer)
	{
		m_threadMixer->stop();
		ThreadManager::getInstance().destroy(m_threadMixer);
	}

	m_driver->destroy();

	while (!m_submitQueue.empty())
	{
		freeAlign(m_submitQueue.back().samples[0]);
		m_submitQueue.pop_back();
	}

	while (!m_samplesBlocks.empty())
	{
		freeAlign(m_samplesBlocks.back());
		m_samplesBlocks.pop_back();
	}
}

void SoundSystem::setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount])
{
	std::memcpy(m_desc.cm, cm, sizeof(cm));
}

SoundChannel* SoundSystem::getChannel(uint32_t channelId)
{
	T_ASSERT (channelId < m_channels.size());
	return m_channels[channelId];
}

SoundChannel* SoundSystem::playSound(uint32_t channelId, Sound* sound, uint32_t repeat)
{
	T_ASSERT (channelId < m_channels.size());
	Acquire< Semaphore > lock(m_channelAttachLock);
	m_channels[channelId]->playSound(sound, m_time, repeat);
	return m_channels[channelId];
}

SoundChannel* SoundSystem::playSound(Sound* sound, bool wait, uint32_t repeat)
{
	for (;;)
	{
		// Allocate first idle channel.
		{
			Acquire< Semaphore > lock(m_channelAttachLock);
			for (RefArray< SoundChannel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
			{
				if (!(*i)->isPlaying())
				{
					(*i)->playSound(sound, m_time, repeat);
					return *i;
				}
			}
		}

		if (!wait)
			break;

		// Yield calling thread; @fixme Notification when channel is free.
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
	}
	return 0;
}

void SoundSystem::stopSound(uint32_t channelId)
{
	T_ASSERT (channelId < m_channels.size());
	Acquire< Semaphore > lock(m_channelAttachLock);
	m_channels[channelId]->stop();
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
	SoundBlock requestBlock;
	SoundBlock frameBlock;
	Timer timerMixer;

	timerMixer.start();

	while (!m_threadMixer->stopped())
	{
		// Wait until submission queue is below threshold.
		while (m_submitQueue.size() >= c_mixerFramesAhead && !m_threadMixer->stopped())
			m_submitConsumedEvent.wait(100);

		if (m_threadMixer->stopped())
			break;

		double startTime = timerMixer.getElapsedTime();

		// Allocate new frame block.
		m_samplesBlocksLock.acquire();
		T_ASSERT_M(!m_samplesBlocks.empty(), L"Out of sample blocks");
		float* samples = m_samplesBlocks.back();
		m_samplesBlocks.pop_back();
		m_samplesBlocksLock.release();

		// Prepare new frame block.
		std::memset (samples, 0, m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels * sizeof(float));
		for (uint32_t i = 0; i < m_desc.driverDesc.hwChannels; ++i)
			frameBlock.samples[i] = samples + m_desc.driverDesc.frameSamples * i;
		frameBlock.samplesCount = m_desc.driverDesc.frameSamples;
		frameBlock.sampleRate = m_desc.driverDesc.sampleRate;
		frameBlock.channels = m_desc.driverDesc.hwChannels;

		// Read blocks from channels.
		uint32_t channelsCount = uint32_t(m_channels.size());
		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			requestBlock.samplesCount = m_desc.driverDesc.frameSamples;

			// Temporarily lock channels as we don't want user to attach new sounds just yet.
			m_channelAttachLock.acquire();
			bool got = m_channels[i]->getBlock(m_time, requestBlock);
			m_channelAttachLock.release();
			if (!got)
				continue;

			T_ASSERT (requestBlock.sampleRate == m_desc.driverDesc.sampleRate);
			T_ASSERT (requestBlock.samplesCount <= m_desc.driverDesc.frameSamples);

			// Final combine channels into hardware channels using "combine matrix".
			for (uint32_t j = 0; j < m_desc.driverDesc.hwChannels; ++j)
			{
				for (uint32_t k = 0; k < SbcMaxChannelCount; ++k)
				{
					float strength = m_desc.cm[j][k];
					if (requestBlock.samples[k] && abs(strength) >= FUZZY_EPSILON)
					{
						for (uint32_t m = 0; m < requestBlock.samplesCount; ++m)
							frameBlock.samples[j][m] += requestBlock.samples[k][m] * strength;
					}
				}
			}
		}
		m_time += double(m_desc.driverDesc.frameSamples) / m_desc.driverDesc.sampleRate;

		m_submitQueueLock.acquire();
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

	while (!m_threadSubmit->stopped())
	{
		m_submitQueueLock.acquire();
		if (m_submitQueue.empty())
		{
			log::warning << L"Sound - submit thread starved, waiting for mixer to catch up" << Endl;
			while (m_submitQueue.size() < c_mixerFramesAhead && !m_threadSubmit->stopped())
			{
				m_submitQueueLock.release();
				m_submitQueueEvent.wait(100);
				m_submitQueueLock.acquire();
			}
			if (m_threadSubmit->stopped())
				break;
		}

		double startTime = timerSubmit.getElapsedTime();

		// Submit block to driver.
		m_driver->submit(m_submitQueue.front());

		// Move block back into heap.
		m_samplesBlocksLock.acquire();
		m_samplesBlocks.push_back(m_submitQueue.front().samples[0]);
		m_samplesBlocksLock.release();
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
