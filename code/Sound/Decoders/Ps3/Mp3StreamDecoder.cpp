/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <mad.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Decoders/Mp3StreamDecoder.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

// Convert from MAD fixed point to 32-bit float sample.
inline float scale(mad_fixed_t sample)
{
	double output = mad_f_todouble(sample);
	return float(output);
}

		}

class Mp3StreamDecoderImpl : public Object
{
public:
	enum
	{
		DecodedBufferSamples = 65536,
		DecodedBufferSize = DecodedBufferSamples * sizeof(float)
	};

	Mp3StreamDecoderImpl()
	:	m_decodedCount(0)
	,	m_consumedCount(0)
	,	m_sampleRate(0)
	{
	}

	bool create(IStream* stream)
	{
		m_stream = stream;

		m_decoded[0] = (float*)Alloc::acquireAlign(DecodedBufferSize, 16, T_FILE_LINE);
		m_decoded[1] = (float*)Alloc::acquireAlign(DecodedBufferSize, 16, T_FILE_LINE);

		if (!m_decoded[0] || !m_decoded[1])
			return false;


		return true;
	}

	void destroy()
	{
		mad_synth_finish(&m_mad_synth);
		mad_frame_finish(&m_mad_frame);
		mad_stream_finish(&m_mad_stream);

		Alloc::freeAlign(m_decoded[0]);
		Alloc::freeAlign(m_decoded[1]);
	}

	void reset()
	{
		mad_timer_reset(&m_mad_timer);

		mad_synth_finish(&m_mad_synth);
		mad_frame_finish(&m_mad_frame);
		mad_stream_finish(&m_mad_stream);

		mad_stream_init(&m_mad_stream);
		mad_frame_init(&m_mad_frame);
		mad_synth_init(&m_mad_synth);

		m_decodedCount = 0;
		m_consumedCount = 0;
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(SoundBlock& outSoundBlock)
	{
		if (m_consumedCount)
		{
			T_ASSERT (m_consumedCount <= m_decodedCount);
			std::memmove(m_decoded[SbcLeft], &m_decoded[SbcLeft][m_consumedCount], (m_decodedCount - m_consumedCount) * sizeof(float));
			std::memmove(m_decoded[SbcRight], &m_decoded[SbcRight][m_consumedCount], (m_decodedCount - m_consumedCount) * sizeof(float));
			m_decodedCount -= m_consumedCount;
			m_consumedCount = 0;
		}

		while (
			(m_decodedCount < outSoundBlock.samplesCount || m_sampleRate == 0) &&
			m_decodedCount < sizeof_array(m_decoded[0]) 
		)
		{
			if (!m_mad_stream.buffer || m_mad_stream.error == MAD_ERROR_BUFLEN)
			{
				uint32_t remaining = 0;
				uint32_t readSize = 0;
				uint8_t* readStart = 0;

				if (m_mad_stream.next_frame)
				{
					remaining = uint32_t(m_mad_stream.bufend - m_mad_stream.next_frame);
					std::memmove(m_readBuffer, m_mad_stream.next_frame, remaining);
					readStart = &m_readBuffer[remaining];
					readSize = sizeof(m_readBuffer) - remaining;
				}
				else
				{
					remaining = 0;
					readStart = m_readBuffer;
					readSize = sizeof(m_readBuffer);
				}

				int readStream = m_stream->read(readStart, readSize);
				if (readStream <= 0)
					return false;

				mad_stream_buffer(&m_mad_stream, m_readBuffer, readStream + remaining);

				m_mad_stream.error = MAD_ERROR_NONE;
			}

			if (mad_frame_decode(&m_mad_frame, &m_mad_stream))
			{
				if (MAD_RECOVERABLE(m_mad_stream.error) || m_mad_stream.error == MAD_ERROR_BUFLEN)
					continue;
				else
				{
					log::error << L"Sound - Unrecoverable MP3 decode error" << Endl;
					return false;
				}
			}

			mad_timer_add(&m_mad_timer, m_mad_frame.header.duration);
			mad_synth_frame(&m_mad_synth, &m_mad_frame);

			if (m_mad_synth.pcm.samplerate != 0)
				m_sampleRate = m_mad_synth.pcm.samplerate;

			const mad_fixed_t* left = m_mad_synth.pcm.samples[0];
			const mad_fixed_t* right = m_mad_synth.pcm.samples[1];

			if (m_mad_synth.pcm.channels >= 2)
			{
				T_ASSERT (m_mad_synth.pcm.length + m_decodedCount < DecodedBufferSamples);
				for (uint32_t i = 0; i < m_mad_synth.pcm.length; ++i)
				{
					m_decoded[SbcLeft][m_decodedCount] = scale(*left++);
					m_decoded[SbcRight][m_decodedCount] = scale(*right++);
					m_decodedCount++;
				}
			}
			else if (m_mad_synth.pcm.channels >= 1)
			{
				T_ASSERT (m_mad_synth.pcm.length + m_decodedCount < DecodedBufferSamples);
				for (uint32_t i = 0; i < m_mad_synth.pcm.length; ++i)
				{
					m_decoded[SbcLeft][m_decodedCount] = scale(*left++);
					m_decoded[SbcRight][m_decodedCount] = 0.0f;
					m_decodedCount++;
				}
			}
			else
			{
				T_ASSERT (m_mad_synth.pcm.length + m_decodedCount < DecodedBufferSamples);
				for (uint32_t i = 0; i < m_mad_synth.pcm.length; ++i)
				{
					m_decoded[SbcLeft][m_decodedCount] = 0.0f;
					m_decoded[SbcRight][m_decodedCount] = 0.0f;
					m_decodedCount++;
				}
			}
		}

		outSoundBlock.samples[SbcLeft] = m_decoded[SbcLeft];
		outSoundBlock.samples[SbcRight] = m_decoded[SbcRight];
		outSoundBlock.samplesCount = alignDown(std::min(m_decodedCount, outSoundBlock.samplesCount), 4);
		outSoundBlock.sampleRate = m_sampleRate;
		outSoundBlock.maxChannel = 2;

		m_consumedCount = outSoundBlock.samplesCount;

		if (!outSoundBlock.sampleRate)
			return false;

		return true;
	}

private:
	Ref< IStream > m_stream;
	mad_stream m_mad_stream;
	mad_frame m_mad_frame;
	mad_synth m_mad_synth;
	mad_timer_t m_mad_timer;
	uint8_t m_readBuffer[8192];
	float* m_decoded[2];
	uint32_t m_decodedCount;
	uint32_t m_consumedCount;
	uint32_t m_sampleRate;
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Mp3StreamDecoder", 0, Mp3StreamDecoder, IStreamDecoder)

bool Mp3StreamDecoder::create(IStream* stream)
{
	if ((m_stream = stream) == 0)
		return false;

	m_decoderImpl = new Mp3StreamDecoderImpl();
	if (!m_decoderImpl->create(m_stream))
	{
		m_decoderImpl = 0;
		m_stream = 0;
		return false;
	}

	return true;
}

void Mp3StreamDecoder::destroy()
{
	safeDestroy(m_decoderImpl);
}

double Mp3StreamDecoder::getDuration() const
{
	T_ASSERT (m_decoderImpl);
	return m_decoderImpl->getDuration();
}

bool Mp3StreamDecoder::getBlock(SoundBlock& outSoundBlock)
{
	T_ASSERT (m_decoderImpl);
	return m_decoderImpl->getBlock(outSoundBlock);
}

void Mp3StreamDecoder::rewind()
{
	T_ASSERT (m_decoderImpl);
	m_stream->seek(IStream::SeekSet, 0);
	m_decoderImpl->reset();
}

	}
}
