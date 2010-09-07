#include <cmath>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundChannel", SoundChannel, Object)

SoundChannel::SoundChannel(uint32_t id, Event& eventFinish, uint32_t hwSampleRate, uint32_t hwFrameSamples)
:	m_id(id)
,	m_eventFinish(eventFinish)
,	m_hwSampleRate(hwSampleRate)
,	m_hwFrameSamples(hwFrameSamples)
,	m_priority(0)
,	m_repeat(0)
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_filter == filter)
		return;

	if ((m_filter = filter) != 0)
		m_filterInstance = filter->createInstance();
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
	return bool(m_sound != 0);
}

void SoundChannel::stop()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_sound = 0;
	m_cursor = 0;
	m_eventFinish.broadcast();
}

ISoundBufferCursor* SoundChannel::getCursor() const
{
	return m_cursor;
}

bool SoundChannel::playSound(const Sound* sound, double time, uint32_t priority, uint32_t repeat)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

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

	m_cursor = soundBuffer->createCursor();
	if (!m_cursor)
	{
		log::error << L"playSound failed; unable to create cursor" << Endl;
		return false;
	}

	m_sound = sound;
	m_priority = priority;
	m_repeat = max< uint32_t >(repeat, 1U);
	m_outputSamplesIn = 0;

	return true;
}

bool SoundChannel::getBlock(const ISoundMixer* mixer, double time, SoundBlock& outBlock)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_sound || !m_cursor)
		return false;

	ISoundBuffer* soundBuffer = m_sound->getSoundBuffer();
	T_ASSERT (soundBuffer);

	// Remove old output samples.
	if (m_outputSamplesIn >= m_hwFrameSamples)
	{
		m_outputSamplesIn -= m_hwFrameSamples;
		if (m_outputSamplesIn > 0)
		{
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
				std::memmove(
					m_outputSamples[i],
					m_outputSamples[i] + m_hwFrameSamples,
					m_outputSamplesIn * sizeof(float)
				);
		}
	}

	while (m_outputSamplesIn < m_hwFrameSamples)
	{
		// Request sound block from buffer.
		SoundBlock soundBlock = { { 0, 0, 0, 0, 0, 0, 0, 0 }, m_hwFrameSamples, 0, 0 };
		if (!soundBuffer->getBlock(m_cursor, soundBlock))
		{
			// No more blocks from sound buffer.
			if (--m_repeat > 0)
			{
				m_cursor->reset();
				if (!soundBuffer->getBlock(m_cursor, soundBlock))
				{
					m_sound = 0;
					m_cursor = 0;
					m_eventFinish.broadcast();
					return false;
				}
			}
			else
			{
				m_sound = 0;
				m_cursor = 0;
				m_eventFinish.broadcast();
				return false;
			}
		}

		// We might get a null block; does not indicate end of stream.
		if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
			return false;

		// Apply filter on sound block.
		if (m_filter)
			m_filter->apply(m_filterInstance, soundBlock);

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
	return m_priority;
}

	}
}
