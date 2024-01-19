/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Timer/Timer.h"
#include "Sound/AudioChannel.h"
#include "Sound/IAudioMixer.h"
#include "Sound/Sound.h"
#include "Sound/Tracker/IEvent.h"
#include "Sound/Tracker/Pattern.h"
#include "Sound/Tracker/Play.h"
#include "Sound/Tracker/SongBuffer.h"
#include "Sound/Tracker/Track.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const int32_t c_maxTrackCount = 16;

class SoundBufferCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	float* m_outputSamples[SbcMaxChannelCount];
	Timer m_timer;
	int32_t m_bpm;
	int32_t m_currentPattern;
	int32_t m_currentRow;
	Ref< AudioChannel > m_channels[c_maxTrackCount];

	SoundBufferCursor()
	:	m_bpm(0)
	,	m_currentPattern(0)
	,	m_currentRow(-1)
	{
		m_outputSamples[0] = nullptr;
	}

	virtual ~SoundBufferCursor()
	{
		Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter)
	{
	}

	virtual void disableRepeat()
	{
	}

	virtual void reset()
	{
		m_currentPattern = 0;
		m_currentRow = -1;
	}
};

const uint32_t c_outputSamplesBlockCount = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SongBuffer", SongBuffer, IAudioBuffer)

SongBuffer::SongBuffer(const RefArray< Pattern >& patterns, int32_t bpm)
:	m_patterns(patterns)
,	m_bpm(bpm)
{
}

Ref< IAudioBufferCursor > SongBuffer::createCursor() const
{
	Ref< SoundBufferCursor > soundBufferCursor = new SoundBufferCursor();

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	soundBufferCursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		soundBufferCursor->m_outputSamples[i] = soundBufferCursor->m_outputSamples[0] + outputSamplesCount * i;

	for (uint32_t i = 0; i < c_maxTrackCount; ++i)
		soundBufferCursor->m_channels[i] = new AudioChannel(
			i,
			44100,
			64
		);

	soundBufferCursor->m_bpm = m_bpm;
	soundBufferCursor->m_currentPattern = 0;
	soundBufferCursor->m_currentRow = -1;

	return soundBufferCursor;
}

bool SongBuffer::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	SoundBufferCursor* soundBufferCursor = static_cast< SoundBufferCursor* >(cursor);
	while (soundBufferCursor->m_currentPattern < m_patterns.size())
	{
		const Pattern* currentPattern = m_patterns[soundBufferCursor->m_currentPattern];
		const int32_t position = int32_t(3.5 * soundBufferCursor->m_bpm * soundBufferCursor->m_timer.getElapsedTime() / 60.0);

		if (position < currentPattern->getDuration())
		{
			const auto& tracks = currentPattern->getTracks();
			if (position != soundBufferCursor->m_currentRow)
			{
				soundBufferCursor->m_currentRow = position;

				for (size_t i = 0; i < tracks.size(); ++i)
				{
					const Track::Key* key = tracks[i]->findKey(position);
					if (!key)
						continue;

					auto& ch = soundBufferCursor->m_channels[i];

					if (key->play)
					{
						const auto& p = key->play;
						ch->play(
							p->getSound()->getBuffer(),
							0,
							0.0f,
							key->play->getRepeatLength() > 0,
							key->play->getRepeatFrom()
						);

						const double pitch = std::pow(1.059463094, p->getNote() - 57);
						ch->setPitch(pitch);

						ch->setVolume(1.0f);
					}

					for (const auto& event : key->events)
						event->execute(ch, soundBufferCursor->m_bpm, soundBufferCursor->m_currentPattern, soundBufferCursor->m_currentRow);
				}
			}

			outBlock.sampleRate = 0;
			outBlock.samplesCount = 0;
			outBlock.maxChannel = 0;

			for (size_t i = 0; i < c_maxTrackCount; ++i)
			{
				auto& ch = soundBufferCursor->m_channels[i];
				if (!ch->isPlaying())
					continue;

				AudioBlock soundBlock = { 0 };
				if (ch->getBlock(mixer, soundBlock))
				{
					outBlock.sampleRate = max(outBlock.sampleRate, soundBlock.sampleRate);
					outBlock.samplesCount = max(outBlock.samplesCount, soundBlock.samplesCount);
					outBlock.maxChannel = max(outBlock.maxChannel, soundBlock.maxChannel);
					for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
					{
						if (soundBlock.samples[j])
						{
							if (outBlock.samples[j])
							{
								mixer->addMulConst(
									outBlock.samples[j],
									soundBlock.samples[j],
									soundBlock.samplesCount,
									1.0f
								);
							}
							else
							{
								outBlock.samples[j] = soundBufferCursor->m_outputSamples[j];
								mixer->mulConst(
									outBlock.samples[j],
									soundBlock.samples[j],
									soundBlock.samplesCount,
									1.0f
								);
							}
						}
					}
				}
			}

			break;
		}
		else
		{
			soundBufferCursor->m_currentPattern++;
			soundBufferCursor->m_currentRow = -1;
			soundBufferCursor->m_timer.reset();
		}
	}
	return bool(soundBufferCursor->m_currentPattern < m_patterns.size());
}

int32_t SongBuffer::getCurrentPattern(const IAudioBufferCursor* cursor) const
{
	const SoundBufferCursor* soundBufferCursor = static_cast< const SoundBufferCursor* >(cursor);
	return soundBufferCursor->m_currentPattern;
}

int32_t SongBuffer::getCurrentRow(const IAudioBufferCursor* cursor) const
{
	const SoundBufferCursor* soundBufferCursor = static_cast< const SoundBufferCursor* >(cursor);
	return soundBufferCursor->m_currentRow;
}

	}
}
