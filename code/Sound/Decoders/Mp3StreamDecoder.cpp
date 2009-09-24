#include <cstring>
#include <mad.h>
#include "Sound/Decoders/Mp3StreamDecoder.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

// Convert from MAD fixed point to 32-bit float sample.
inline float scale(mad_fixed_t sample)
{
	double output = double(sample) / (1L << MAD_F_FRACBITS);
	return float(output);
}

		}

class Mp3StreamDecoderImpl : public Object
{
public:
	Mp3StreamDecoderImpl()
	:	m_decodedCount(0)
	,	m_consumedCount(0)
	{
	}

	bool create(Stream* stream)
	{
		m_stream = stream;

		mad_stream_init(&m_mad_stream);
		mad_frame_init(&m_mad_frame);
		mad_synth_init(&m_mad_synth);
		mad_timer_reset(&m_mad_timer);

		return true;
	}

	void destroy()
	{
		mad_synth_finish(&m_mad_synth);
		mad_frame_finish(&m_mad_frame);
		mad_stream_finish(&m_mad_stream);
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
			memmove(m_decoded[SbcLeft], &m_decoded[SbcLeft][m_consumedCount], (m_decodedCount - m_consumedCount) * sizeof(float));
			memmove(m_decoded[SbcRight], &m_decoded[SbcRight][m_consumedCount], (m_decodedCount - m_consumedCount) * sizeof(float));
			m_decodedCount -= m_consumedCount;
			m_consumedCount = 0;
		}

		while (m_decodedCount < outSoundBlock.samplesCount)
		{
			if (!m_mad_stream.buffer || m_mad_stream.error == MAD_ERROR_BUFLEN)
			{
				uint32_t remaining = 0;
				uint32_t readSize = 0;
				uint8_t* readStart = 0;

				if (m_mad_stream.next_frame)
				{
					remaining = uint32_t(m_mad_stream.bufend - m_mad_stream.next_frame);
					memmove(m_readBuffer, m_mad_stream.next_frame, remaining);
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

				log::error << L"Sound - Unrecoverable MP3 decode error" << Endl;
				return false;
			}

			mad_timer_add(&m_mad_timer, m_mad_frame.header.duration);
			mad_synth_frame(&m_mad_synth, &m_mad_frame);

			T_ASSERT (outSoundBlock.sampleRate == 0 || outSoundBlock.sampleRate == m_mad_synth.pcm.samplerate);
			outSoundBlock.sampleRate = m_mad_synth.pcm.samplerate;
			outSoundBlock.maxChannel = m_mad_synth.pcm.channels;

			const mad_fixed_t* left = m_mad_synth.pcm.samples[0];
			const mad_fixed_t* right = m_mad_synth.pcm.samples[1];

			for (uint32_t i = 0; i < m_mad_synth.pcm.length; ++i)
			{
				T_ASSERT (m_decodedCount < sizeof_array(m_decoded[0]));
				m_decoded[SbcLeft][m_decodedCount] = scale(*left++);
				if (m_mad_synth.pcm.channels == 2)
					m_decoded[SbcRight][m_decodedCount] = scale(*right++);
				m_decodedCount++;
			}
		}

		outSoundBlock.samples[SbcLeft] = m_decoded[SbcLeft];
		outSoundBlock.samples[SbcRight] = m_decoded[SbcRight];
		outSoundBlock.samplesCount = std::min(m_decodedCount, outSoundBlock.samplesCount);

		m_consumedCount = outSoundBlock.samplesCount;

		if (!outSoundBlock.sampleRate)
			return false;

		return true;
	}

private:
	Ref< Stream > m_stream;
	mad_stream m_mad_stream;
	mad_frame m_mad_frame;
	mad_synth m_mad_synth;
	mad_timer_t m_mad_timer;
	uint8_t m_readBuffer[8192];
	float m_decoded[2][65535];
	uint32_t m_decodedCount;
	uint32_t m_consumedCount;
};

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.Mp3StreamDecoder", Mp3StreamDecoder, IStreamDecoder)

bool Mp3StreamDecoder::create(Stream* stream)
{
	m_stream = stream;
	rewind();
	return m_decoderImpl != 0;
}

void Mp3StreamDecoder::destroy()
{
	if (m_decoderImpl)
	{
		m_decoderImpl->destroy();
		m_decoderImpl = 0;
	}
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
	destroy();
	m_stream->seek(Stream::SeekSet, 0);
	m_decoderImpl = gc_new< Mp3StreamDecoderImpl >();
	if (!m_decoderImpl->create(m_stream))
		m_decoderImpl = 0;
}

	}
}
