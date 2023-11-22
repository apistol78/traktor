/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Sound/AudioChannel.h"
#include "Sound/IAudioMixer.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"

namespace traktor::sound
{
	namespace
	{

const uint32_t c_outputSamplesBlockCount = 16;

inline void moveSamples(float* destSamples, const float* sourceSamples, int32_t samplesCount)
{
	T_ASSERT((samplesCount & 3) == 0);
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.AudioChannel", AudioChannel, Object)

AudioChannel::AudioChannel(uint32_t id, uint32_t hwSampleRate, uint32_t hwFrameSamples)
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

AudioChannel::~AudioChannel()
{
	Alloc::freeAlign(m_outputSamples[0]);
}

void AudioChannel::setVolume(float volume)
{
	m_volume = clamp(volume, 0.0f, 1.0f);
}

float AudioChannel::getVolume() const
{
	return m_volume;
}

void AudioChannel::setPitch(float pitch)
{
	m_pitch = pitch;
}

float AudioChannel::getPitch() const
{
	return m_pitch;
}

void AudioChannel::setFilter(const IFilter* filter)
{
	StateFilter& sf = m_stateFilter.beginWrite();
	if (filter != nullptr)
	{
		sf.filter = filter;
		sf.filterInstance = filter->createInstance();
	}
	else
	{
		sf.filter = nullptr;
		sf.filterInstance = nullptr;
	}
	m_stateFilter.endWrite();
}

bool AudioChannel::isPlaying() const
{
	return m_playing;
}

void AudioChannel::stop()
{
	StateSound& ss = m_stateSound.beginWrite();

	ss.buffer = nullptr;
	ss.cursor = nullptr;
	ss.category = 0;
	ss.volume = 0.0f;
	ss.repeat = false;
	ss.repeatFrom = 0;

	m_playing = false;
	m_allowRepeat = false;

	m_stateSound.endWrite();
}

ISoundBufferCursor* AudioChannel::getCursor()
{
	return m_stateSound.read().cursor;
}

void AudioChannel::setParameter(handle_t id, float parameter)
{
	StateParameter& sp = m_stateParameters.beginWrite();
	sp.set.push_back(std::make_pair(id, parameter));
	m_stateParameters.endWrite();
}

void AudioChannel::disableRepeat()
{
	m_allowRepeat = false;
}

bool AudioChannel::play(
	const ISoundBuffer* buffer,
	handle_t category,
	float gain,
	bool repeat,
	uint32_t repeatFrom
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
	ss.repeat = repeat;
	ss.repeatFrom = repeatFrom;

	m_allowRepeat = true;
	m_playing = true;

	m_stateSound.endWrite();
	return true;
}

bool AudioChannel::getBlock(const IAudioMixer* mixer, SoundBlock& outBlock)
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
	T_ASSERT(soundBuffer);

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
			if (m_allowRepeat && ss.repeat)
			{
				ss.cursor->reset();

				// Skip samples when repeating.
				uint32_t skip = ss.repeatFrom;
				while (skip > 0)
				{
					SoundBlock skipSoundBlock = { { 0 }, m_hwFrameSamples, 0, 0 };
					if (soundBuffer->getBlock(ss.cursor, mixer, skipSoundBlock))
						skip -= min(skip, skipSoundBlock.samplesCount);
					else
					{
						ss.buffer = nullptr;
						ss.cursor = nullptr;
						m_playing = false;
						return false;
					}
				}

				if (!soundBuffer->getBlock(ss.cursor, mixer, soundBlock))
				{
					ss.buffer = nullptr;
					ss.cursor = nullptr;
					m_playing = false;
					return false;
				}
			}
			else
			{
				ss.buffer = nullptr;
				ss.cursor = nullptr;
				m_playing = false;
				return false;
			}
		}

		// We might get a null block; does not indicate end of stream.
		if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
			return false;

		T_ASSERT(soundBlock.samplesCount <= m_hwFrameSamples);

		// Apply filter on sound block.
		const StateFilter& sf = m_stateFilter.read();
		if (sf.filter)
		{
			sf.filter->apply(sf.filterInstance, soundBlock);
			T_ASSERT(soundBlock.samplesCount <= m_hwFrameSamples);
		}

		const uint32_t sampleRate = uint32_t(m_pitch * soundBlock.sampleRate);

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
				T_ASSERT(alignUp(outputSamples, 16) == outputSamples);
				T_ASSERT(m_outputSamplesIn + outputSamplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

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
				T_ASSERT(m_outputSamplesIn + soundBlock.samplesCount < m_hwFrameSamples * c_outputSamplesBlockCount);

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

	outBlock.category = ss.category;
	return true;
}

}
