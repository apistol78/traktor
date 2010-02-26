#include <cmath>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Thread/Acquire.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Sound.h"
#include "Sound/SoundBlockUtilities.h"
#include "Sound/SoundChannel.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_outputSamplesBlockCount = 4;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundChannel", SoundChannel, Object)

SoundChannel::SoundChannel(uint32_t hwSampleRate, uint32_t hwFrameSamples)
:	m_hwSampleRate(hwSampleRate)
,	m_hwFrameSamples(hwFrameSamples)
,	m_time(0.0)
,	m_repeat(0)
,	m_outputSamplesIn(0)
,	m_volume(1.0f)
{
	const uint32_t outputSamplesCount = hwFrameSamples * c_outputSamplesBlockCount;
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		m_outputSamples[i] = static_cast< float* >(Alloc::acquireAlign(outputSamplesCount * sizeof(float), 16));
}

SoundChannel::~SoundChannel()
{
	for (uint32_t i = 0; i < sizeof_array(m_outputSamples); ++i)
		Alloc::freeAlign(m_outputSamples[i]);
}

void SoundChannel::setVolume(float volume)
{
	m_volume = clamp(volume, 0.0f, 1.0f);
}

void SoundChannel::setFilter(IFilter* filter)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_filterLock);
	if ((m_filter = filter) != 0)
		m_filterInstance = filter->createInstance();
}

Ref< IFilter > SoundChannel::getFilter() const
{
	return m_filter;
}

bool SoundChannel::isPlaying() const
{
	// Assume we're playing if we have an attached sound.
	return bool(m_sound != 0);
}

void SoundChannel::stop()
{
	m_sound = 0;
}

ISoundBufferCursor* SoundChannel::getCursor() const
{
	return m_cursor;
}

void SoundChannel::playSound(Sound* sound, double time, uint32_t repeat)
{
	Ref< ISoundBuffer > soundBuffer = sound->getSoundBuffer();
	if (soundBuffer)
	{
		m_sound = sound;
		m_time = time;
		m_repeat = max< uint32_t >(repeat, 1U);
		m_cursor = soundBuffer->createCursor();
	}
}

bool SoundChannel::getBlock(double time, SoundBlock& outBlock)
{
	if (!m_sound || !m_cursor)
		return false;

	Ref< ISoundBuffer > soundBuffer = m_sound->getSoundBuffer();
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

	// Local sound time.
	double soundTime = time - m_time;
	soundTime = std::max(0.0, soundTime);

	while (m_outputSamplesIn < m_hwFrameSamples)
	{
		// Move cursor to local time.
		m_cursor->setCursor(soundTime);

		// Request sound block from buffer.
		SoundBlock soundBlock = { { 0, 0 }, m_hwFrameSamples, 0, 0 };
		if (!soundBuffer->getBlock(m_cursor, soundBlock))
		{
			// No more blocks from sound buffer.
			if (--m_repeat > 0)
			{
				m_cursor->setCursor(0.0);
				if (!soundBuffer->getBlock(m_cursor, soundBlock))
				{
					m_sound = 0;
					return false;
				}
				m_time = time;
			}
			else
			{
				m_sound = 0;
				return false;
			}
		}

		// We might get a null block; does not indicate end of stream.
		if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
			return false;

		soundTime += double(soundBlock.samplesCount) / soundBlock.sampleRate;

		// Apply filter on sound block.
		if (m_filter)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_filterLock);
			m_filter->apply(m_filterInstance, soundBlock);
		}

		// Convert sound block into hardware required sample rate.
		if (soundBlock.sampleRate != m_hwSampleRate)
		{
			uint32_t outputSamplesCount = uint32_t(double(soundBlock.samplesCount * m_hwSampleRate) / soundBlock.sampleRate + 0.5);
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			{
				const float* inputSamples = soundBlock.samples[i];
				float* outputSamples = m_outputSamples[i] + m_outputSamplesIn;
				if (inputSamples)
				{
					float halfVolume = m_volume * 0.5f;
					uint32_t p0 = 0;
					for (uint32_t j = 0; j < outputSamplesCount; ++j)
					{
						uint32_t p1 = ((j + 1) * soundBlock.sampleRate) / m_hwSampleRate;
						outputSamples[j] = (inputSamples[p0] + inputSamples[p1]) * halfVolume;
						p0 = p1;
					}
				}
				else
					soundBlockMute(outputSamples, soundBlock.samplesCount);
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
					soundBlockMulConst(outputSamples, inputSamples, soundBlock.samplesCount, m_volume);
				else
					soundBlockMute(outputSamples, soundBlock.samplesCount);
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

	}
}
