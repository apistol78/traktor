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
#include "Sound/SoundChannel.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_outputSamplesBlockCount = 8;

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

SoundChannel::~SoundChannel()
{
	Alloc::freeAlign(m_outputSamples[0]);
}

handle_t SoundChannel::getCategory() const
{
	return m_state.category;
}

void SoundChannel::setVolume(float volume)
{
	m_volume = clamp(volume, 0.0f, 1.0f);
}

void SoundChannel::setFilter(const IFilter* filter)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_state.filter != filter)
	{
		if ((m_state.filter = filter) != 0)
			m_state.filterInstance = filter->createInstance();
		else
			m_state.filterInstance = 0;
	}
}

const IFilter* SoundChannel::getFilter() const
{
	return m_state.filter;
}

void SoundChannel::setPitch(float pitch)
{
	m_state.pitch = pitch;
}

float SoundChannel::getPitch() const
{
	return m_state.pitch;
}

bool SoundChannel::isPlaying() const
{
	return bool(m_state.buffer != 0);
}

void SoundChannel::stop()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	m_state.buffer = 0;
	m_state.cursor = 0;
	m_state.filter = 0;
	m_state.filterInstance = 0;

	m_eventFinish.broadcast();
}

ISoundBufferCursor* SoundChannel::getCursor() const
{
	return m_state.cursor;
}

void SoundChannel::setParameter(handle_t id, float parameter)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_state.cursor)
		m_state.cursor->setParameter(id, parameter);
}

void SoundChannel::disableRepeat()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_state.cursor)
		m_state.cursor->disableRepeat();
}

bool SoundChannel::play(
	const ISoundBuffer* buffer,
	handle_t category,
	float volume,
	float presence,
	float presenceRate,
	uint32_t repeat
)
{
	if (!buffer)
		return false;

	Ref< ISoundBufferCursor > cursor = buffer->createCursor();
	if (!cursor)
		return false;

	// Queue state; activated next time channel is polled for another
	// sound block.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_state.buffer = buffer;
		m_state.cursor = cursor;
		m_state.category = category;
		m_state.volume = volume;
		m_state.pitch = 1.0f;
		m_state.presence = presence;
		m_state.presenceRate = presenceRate;
		m_state.repeat = max< uint32_t >(repeat, 1);
	}

	return true;
}

SoundChannel::SoundChannel(uint32_t id, Event& eventFinish, uint32_t hwSampleRate, uint32_t hwFrameSamples)
:	m_id(id)
,	m_eventFinish(eventFinish)
,	m_hwSampleRate(hwSampleRate)
,	m_hwFrameSamples(hwFrameSamples)
,	m_volume(1.0f)
,	m_outputSamplesIn(0)
{
	const uint32_t outputSamplesCount = hwFrameSamples * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		m_outputSamples[i] = m_outputSamples[0] + outputSamplesCount * i;
}

bool SoundChannel::getBlock(const ISoundMixer* mixer, double time, SoundBlock& outBlock)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_state.buffer || !m_state.cursor)
		return false;

	const ISoundBuffer* soundBuffer = m_state.buffer;
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

	// Build output block.
	outBlock.samplesCount = m_hwFrameSamples;
	outBlock.sampleRate = m_hwSampleRate;
	outBlock.maxChannel = 0;

	while (m_outputSamplesIn < m_hwFrameSamples)
	{
		// Request sound block from buffer.
		SoundBlock soundBlock = { { 0, 0, 0, 0, 0, 0, 0, 0 }, m_hwFrameSamples, 0, 0 };
		if (!soundBuffer->getBlock(m_state.cursor, mixer, soundBlock))
		{
			// No more blocks from sound buffer.
			if (--m_state.repeat > 0)
			{
				m_state.cursor->reset();
				if (!soundBuffer->getBlock(m_state.cursor, mixer, soundBlock))
				{
					m_state.buffer = 0;
					m_state.cursor = 0;
					m_state.buffer = 0;
					m_state.cursor = 0;
					m_eventFinish.broadcast();
					return false;
				}
			}
			else
			{
				m_state.buffer = 0;
				m_state.cursor = 0;
				m_state.buffer = 0;
				m_state.cursor = 0;
				m_eventFinish.broadcast();
				return false;
			}
		}

		// We might get a null block; does not indicate end of stream.
		if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
			return false;

		T_ASSERT (soundBlock.samplesCount <= m_hwFrameSamples);

		// Apply filter on sound block.
		if (m_state.filter)
		{
			m_state.filter->apply(m_state.filterInstance, soundBlock);
			T_ASSERT (soundBlock.samplesCount <= m_hwFrameSamples);
		}

		uint32_t sampleRate = uint32_t(m_state.pitch * soundBlock.sampleRate);

		// Convert sound block into hardware required sample rate.
		if (sampleRate != m_hwSampleRate)
		{
			uint32_t outputSamplesCount = (soundBlock.samplesCount * m_hwSampleRate) / sampleRate;

			// Ensure we produce "multiple-of-4" number of samples; slight adjust block's sample rate.
			outputSamplesCount = alignUp(outputSamplesCount, 4);
			soundBlock.sampleRate = (soundBlock.samplesCount * m_hwSampleRate) / outputSamplesCount;

			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			{
				const float* inputSamples = soundBlock.samples[i];
				if (inputSamples)
				{
					float* outputSamples = m_outputSamples[i] + m_outputSamplesIn;
					T_ASSERT (alignUp(outputSamples, 16) == outputSamples);
					T_ASSERT (m_outputSamplesIn + outputSamplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

					mixer->stretch(
						outputSamples,
						outputSamplesCount,
						inputSamples,
						soundBlock.samplesCount,
						m_volume * m_state.volume
					);

					outBlock.samples[i] = m_outputSamples[i];
					outBlock.maxChannel = max(outBlock.maxChannel, i + 1);
				}
			}

			m_outputSamplesIn += outputSamplesCount;
		}
		else
		{
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			{
				const float* inputSamples = soundBlock.samples[i];
				if (inputSamples)
				{
					float* outputSamples = m_outputSamples[i] + m_outputSamplesIn;
					T_ASSERT (m_outputSamplesIn + soundBlock.samplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

					mixer->mulConst(
						outputSamples,
						inputSamples,
						soundBlock.samplesCount,
						m_volume * m_state.volume
					);

					outBlock.samples[i] = m_outputSamples[i];
					outBlock.maxChannel = max(outBlock.maxChannel, i + 1);
				}
			}
			m_outputSamplesIn += soundBlock.samplesCount;
		}
	}

	return true;
}

	}
}
