#include <cmath>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Acquire.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_outputSamplesBlockCount = 3;

inline void moveSamples(float* destSamples, const float* sourceSamples, int32_t samplesCount)
{
	T_ASSERT ((samplesCount & 3) == 0);
	Vector4 tmp[4];
	int32_t i;

	for (i = 0; i < samplesCount - 16; i += 16)
	{
		tmp[0] = Vector4::loadAligned(&sourceSamples[i + 0]);
		tmp[1] = Vector4::loadAligned(&sourceSamples[i + 4]);
		tmp[2] = Vector4::loadAligned(&sourceSamples[i + 8]);
		tmp[3] = Vector4::loadAligned(&sourceSamples[i + 12]);

		tmp[0].storeAligned(&destSamples[i + 0]);
		tmp[1].storeAligned(&destSamples[i + 4]);
		tmp[2].storeAligned(&destSamples[i + 8]);
		tmp[3].storeAligned(&destSamples[i + 12]);
	}

	for (; i < samplesCount; i += 4)
		Vector4::loadAligned(&sourceSamples[i]).storeAligned(&destSamples[i]);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundChannel", SoundChannel, Object)

SoundChannel::SoundChannel(uint32_t id, Event& eventFinish, uint32_t hwSampleRate, uint32_t hwFrameSamples)
:	m_id(id)
,	m_eventFinish(eventFinish)
,	m_hwSampleRate(hwSampleRate)
,	m_hwFrameSamples(hwFrameSamples)
,	m_outputSamplesIn(0)
,	m_volume(1.0f)
,	m_exclusive(false)
{
	const uint32_t outputSamplesCount = hwFrameSamples * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		m_outputSamples[i] = m_outputSamples[0] + outputSamplesCount * i;
}

SoundChannel::~SoundChannel()
{
	Alloc::freeAlign(m_outputSamples[0]);
}

void SoundChannel::setVolume(float volume)
{
	m_volume = clamp(volume, 0.0f, 1.0f);
}

void SoundChannel::setFilter(IFilter* filter)
{
	if (m_filter != filter)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if ((m_filter = filter) != 0)
			m_filterInstance = filter->createInstance();
	}
}

IFilter* SoundChannel::getFilter() const
{
	return m_filter;
}

void SoundChannel::setExclusive(bool exclusive)
{
	m_exclusive = exclusive;
}

bool SoundChannel::isExclusive() const
{
	return m_exclusive;
}

bool SoundChannel::isPlaying() const
{
	return bool(m_currentState.sound != 0);
}

void SoundChannel::stop()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_currentState.sound = 0;
	m_currentState.cursor = 0;
	m_eventFinish.broadcast();
}

ISoundBufferCursor* SoundChannel::getCursor() const
{
	return m_currentState.cursor;
}

bool SoundChannel::playSound(const Sound* sound, double time, uint32_t priority, uint32_t repeat)
{
	if (!sound)
	{
		log::error << L"playSound failed; no sound" << Endl;
		return false;
	}

	ISoundBuffer* soundBuffer = sound->getSoundBuffer();
	if (!soundBuffer)
	{
		log::error << L"playSound failed; no sound buffer" << Endl;
		return false;
	}

	Ref< ISoundBufferCursor > cursor = soundBuffer->createCursor();
	if (!cursor)
	{
		log::error << L"playSound failed; unable to create cursor" << Endl;
		return false;
	}

	// Queue state; activated next time channel is polled for another
	// sound block.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_queuedState.sound = sound;
		m_queuedState.cursor = cursor;
		m_queuedState.priority = priority;
		m_queuedState.repeat = max< uint32_t >(repeat, 1U);
	}

	return true;
}

bool SoundChannel::getBlock(const ISoundMixer* mixer, double time, SoundBlock& outBlock)
{
	// Recover playing state; swap in queued state if necessary.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (m_queuedState.sound)
		{
			m_currentState = m_queuedState;
			m_queuedState.sound = 0;
			m_queuedState.cursor = 0;
			m_outputSamplesIn = 0;
		}
	}

	if (!m_currentState.sound || !m_currentState.cursor)
		return false;

	Ref< ISoundBuffer > soundBuffer = m_currentState.sound->getSoundBuffer();
	T_ASSERT (soundBuffer);

	// Remove old output samples.
	if (m_outputSamplesIn >= m_hwFrameSamples)
	{
		m_outputSamplesIn -= m_hwFrameSamples;
		if (m_outputSamplesIn > 0)
		{
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
				moveSamples(
					m_outputSamples[i],
					m_outputSamples[i] + m_hwFrameSamples,
					alignUp(m_outputSamplesIn, 4)
				);
		}
	}

	while (m_outputSamplesIn < m_hwFrameSamples)
	{
		// Request sound block from buffer.
		SoundBlock soundBlock = { { 0, 0, 0, 0, 0, 0, 0, 0 }, m_hwFrameSamples, 0, 0 };
		if (!soundBuffer->getBlock(m_currentState.cursor, soundBlock))
		{
			// No more blocks from sound buffer.
			if (--m_currentState.repeat > 0)
			{
				m_currentState.cursor->reset();
				if (!soundBuffer->getBlock(m_currentState.cursor, soundBlock))
				{
					m_currentState.sound = 0;
					m_currentState.cursor = 0;
					m_eventFinish.broadcast();
					return false;
				}
			}
			else
			{
				m_currentState.sound = 0;
				m_currentState.cursor = 0;
				m_eventFinish.broadcast();
				return false;
			}
		}

		// We might get a null block; does not indicate end of stream.
		if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
			return false;

		T_ASSERT (soundBlock.samplesCount <= m_hwFrameSamples);

		// Apply filter on sound block.
		if (m_filter)
		{
			m_filter->apply(m_filterInstance, soundBlock);
			T_ASSERT (soundBlock.samplesCount <= m_hwFrameSamples);
		}

		// Convert sound block into hardware required sample rate.
		if (soundBlock.sampleRate != m_hwSampleRate)
		{
			uint32_t outputSamplesCount = (soundBlock.samplesCount * m_hwSampleRate) / soundBlock.sampleRate;

			// Ensure we produce "multiple-of-4" number of samples; slight adjust block's sample rate.
			outputSamplesCount = alignUp(outputSamplesCount, 4);
			soundBlock.sampleRate = (soundBlock.samplesCount * m_hwSampleRate) / outputSamplesCount;

			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			{
				const float* inputSamples = soundBlock.samples[i];
				
				float* outputSamples = m_outputSamples[i] + m_outputSamplesIn;
				T_ASSERT (alignUp(outputSamples, 16) == outputSamples);
				T_ASSERT (m_outputSamplesIn + outputSamplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

				if (inputSamples)
					mixer->stretch(
						outputSamples,
						outputSamplesCount,
						inputSamples,
						soundBlock.samplesCount,
						m_volume
					);
				else
					mixer->mute(outputSamples, outputSamplesCount);
			}

			m_outputSamplesIn += outputSamplesCount;
		}
		else
		{
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			{
				const float* inputSamples = soundBlock.samples[i];
				float* outputSamples = m_outputSamples[i] + m_outputSamplesIn;
				T_ASSERT (m_outputSamplesIn + soundBlock.samplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

				if (inputSamples)
					mixer->mulConst(outputSamples, inputSamples, soundBlock.samplesCount, m_volume);
				else
					mixer->mute(outputSamples, soundBlock.samplesCount);
			}
			m_outputSamplesIn += soundBlock.samplesCount;
		}
	}

	// Build output block.
	outBlock.samplesCount = m_hwFrameSamples;
	outBlock.sampleRate = m_hwSampleRate;
	outBlock.maxChannel = SbcMaxChannelCount;
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		outBlock.samples[i] = m_outputSamples[i];

	return true;
}

uint32_t SoundChannel::getPriority() const
{
	return m_currentState.priority;
}

	}
}
