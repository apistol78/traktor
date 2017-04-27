/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
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

const uint32_t c_outputSamplesBlockCount = 4;

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

void SoundChannel::setVolume(float volume)
{
	m_volume = clamp(volume, 0.0f, 1.0f);
}

float SoundChannel::getVolume() const
{
	return m_volume;
}

void SoundChannel::setPitch(float pitch)
{
	m_pitch = pitch;
}

float SoundChannel::getPitch() const
{
	return m_pitch;
}

void SoundChannel::setFilter(const IFilter* filter)
{
	StateFilter& sf = m_stateFilter.beginWrite();
	if (filter != 0)
	{
		sf.filter = filter;
		sf.filterInstance = filter->createInstance();
	}
	else
	{
		sf.filter = 0;
		sf.filterInstance = 0;
	}
	m_stateFilter.endWrite();
}

bool SoundChannel::isPlaying() const
{
	return m_playing;
}

void SoundChannel::stop()
{
	StateSound& ss = m_stateSound.beginWrite();

	ss.buffer = 0;
	ss.cursor = 0;
	ss.category = 0;
	ss.volume = 0.0f;
	ss.presence = 0.0f;
	ss.presenceRate = 0.0f;
	ss.repeat = 0;

	m_playing = false;
	m_allowRepeat = false;

	m_stateSound.endWrite();
}

ISoundBufferCursor* SoundChannel::getCursor()
{
	return m_stateSound.read().cursor;
}

void SoundChannel::setParameter(handle_t id, float parameter)
{
	StateParameter& sp = m_stateParameters.beginWrite();
	sp.set.push_back(std::make_pair(id, parameter));
	m_stateParameters.endWrite();
}

void SoundChannel::disableRepeat()
{
	m_allowRepeat = false;
}

bool SoundChannel::play(
	const ISoundBuffer* buffer,
	handle_t category,
	float gain,
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

	StateSound& ss = m_stateSound.beginWrite();

	ss.buffer = buffer;
	ss.cursor = cursor;
	ss.category = category;
	ss.volume = decibelToLinear(gain);
	ss.presence = presence;
	ss.presenceRate = presenceRate;
	ss.repeat = max< uint32_t >(repeat, 1);

	m_allowRepeat = true;
	m_playing = true;

	m_stateSound.endWrite();

	return true;
}

SoundChannel::SoundChannel(uint32_t id, uint32_t hwSampleRate, uint32_t hwFrameSamples)
:	m_id(id)
,	m_hwSampleRate(hwSampleRate)
,	m_hwFrameSamples(hwFrameSamples)
,	m_volume(1.0f)
,	m_pitch(1.0f)
,	m_playing(false)
,	m_allowRepeat(false)
,	m_outputSamplesIn(0)
{
	const uint32_t outputSamplesCount = hwFrameSamples * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		m_outputSamples[i] = m_outputSamples[0] + outputSamplesCount * i;
}

bool SoundChannel::getBlock(
	const ISoundMixer* mixer,
	double time,
	SoundBlock& outBlock,
	SoundBlockMeta& outBlockMeta
)
{
	StateSound& ss = m_stateSound.read();

	if (!ss.buffer || !ss.cursor)
		return false;

	if (!m_allowRepeat)
		ss.cursor->disableRepeat();

	// Push pending parameters.
	StateParameter& sp = m_stateParameters.read();
	for (uint32_t i = 0; i < sp.set.size(); ++i)
		ss.cursor->setParameter(sp.set[i].first, sp.set[i].second);
	sp.set.clear();

	const ISoundBuffer* soundBuffer = ss.buffer;
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
		SoundBlock soundBlock = { { 0 }, m_hwFrameSamples, 0, 0 };
		if (!soundBuffer->getBlock(ss.cursor, mixer, soundBlock))
		{
			// No more blocks from sound buffer.
			if (m_allowRepeat && --ss.repeat > 0)
			{
				ss.cursor->reset();
				if (!soundBuffer->getBlock(ss.cursor, mixer, soundBlock))
				{
					ss.buffer = 0;
					ss.cursor = 0;
					m_playing = false;
					return false;
				}
			}
			else
			{
				ss.buffer = 0;
				ss.cursor = 0;
				m_playing = false;
				return false;
			}
		}

		// We might get a null block; does not indicate end of stream.
		if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
			return false;

		T_ASSERT (soundBlock.samplesCount <= m_hwFrameSamples);

		// Apply filter on sound block.
		StateFilter& sf = m_stateFilter.read();
		if (sf.filter)
		{
			sf.filter->apply(sf.filterInstance, soundBlock);
			T_ASSERT (soundBlock.samplesCount <= m_hwFrameSamples);
		}

		uint32_t sampleRate = uint32_t(m_pitch * soundBlock.sampleRate);

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
				
				float* outputSamples = m_outputSamples[i] + m_outputSamplesIn;
				T_ASSERT (alignUp(outputSamples, 16) == outputSamples);
				T_ASSERT (m_outputSamplesIn + outputSamplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

				if (inputSamples)
					mixer->stretch(
						outputSamples,
						outputSamplesCount,
						inputSamples,
						soundBlock.samplesCount,
						m_volume * ss.volume
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
					mixer->mulConst(
						outputSamples,
						inputSamples,
						soundBlock.samplesCount,
						m_volume * ss.volume
					);
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

	outBlockMeta.category = ss.category;
	outBlockMeta.presence = ss.presence;
	outBlockMeta.presenceRate = ss.presenceRate;

	// Only return presence once; sound system manage duck recovery automatically.
	ss.presence = 0.0f;
	ss.presenceRate = 0.0f;

	return true;
}

	}
}
