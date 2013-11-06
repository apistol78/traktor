#include <cstdio>
#include <cstring>
#include <mpg123.h>
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

class Mp3StreamDecoderImpl : public Object
{
public:
	enum
	{
		DecodedBufferSamples = 32768,
		DecodedBufferSize = DecodedBufferSamples * sizeof(float),
		ReadBufferSize = 4096
	};

	Mp3StreamDecoderImpl()
	:	m_handle(0)
	,	m_decodedCount(0)
	,	m_consumedCount(0)
	,	m_sampleRate(0)
	,	m_channels(0)
	,	m_encoding(0)
	{
	}

	bool create(IStream* stream)
	{
		size_t nrates;
		const long* rates;
		int ret;

		m_stream = stream;

		m_decoded[0] = (float*)Alloc::acquireAlign(DecodedBufferSize, 16, T_FILE_LINE);
		m_decoded[1] = (float*)Alloc::acquireAlign(DecodedBufferSize, 16, T_FILE_LINE);

		if (!m_decoded[0] || !m_decoded[1])
			return false;

		if (ms_instances++ <= 0)
			mpg123_init();

		m_handle = mpg123_new(0, &ret);
		if (!m_handle)
			return false;

		ret = mpg123_format_none(m_handle);
		if (ret != MPG123_OK)
			return false;

		mpg123_rates(&rates, &nrates);
		for (size_t i = 0; i < nrates; i++)
		{
			ret = mpg123_format(m_handle, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);
			if (ret != MPG123_OK)
				return false;
		}

		ret = mpg123_open_feed(m_handle);
		if (ret != MPG123_OK)
			return false;

		return true;
	}

	void destroy()
	{
		mpg123_delete(m_handle);

		if (--ms_instances <= 0)
			mpg123_exit();

		Alloc::freeAlign(m_decoded[0]);
		Alloc::freeAlign(m_decoded[1]);
	}

	void reset()
	{
		// Inform mpg123 that we're seeking to the beginning.
		off_t offset = 0;
		mpg123_feedseek(m_handle, 0, SEEK_SET, &offset);

		// Move to correct position in our stream.
		m_stream->seek(IStream::SeekSet, offset);

		// Flush buffers.
		m_decodedCount = 0;
		m_consumedCount = 0;
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(SoundBlock& outSoundBlock)
	{
		// Discard consumed samples; those are lingering in the decoded buffer since last call.
		if (m_consumedCount)
		{
			T_ASSERT (m_consumedCount <= m_decodedCount);
			std::memmove(m_decoded[SbcLeft], &m_decoded[SbcLeft][m_consumedCount], (m_decodedCount - m_consumedCount) * sizeof(float));
			std::memmove(m_decoded[SbcRight], &m_decoded[SbcRight][m_consumedCount], (m_decodedCount - m_consumedCount) * sizeof(float));
			m_decodedCount -= m_consumedCount;
			m_consumedCount = 0;
		}

		// Read and decode until desired amount of samples have been decoded.
		while (m_decodedCount < outSoundBlock.samplesCount || m_sampleRate == 0)
		{
			int32_t nread = m_stream->read(m_readBuffer, sizeof(m_readBuffer));
			if (nread <= 0)
			{
				// No more bytes from source stream; if we have encoded some samples lets output them
				// and return success.
				if (m_decodedCount > 0)
					break;
				return false;
			}

			int32_t ret = mpg123_feed(m_handle, m_readBuffer, nread);
			if (ret == MPG123_NEED_MORE)
				continue;
			if (ret == MPG123_ERR)
				return false;

			while (ret != MPG123_NEED_MORE)
			{
				uint8_t* audio;
				size_t bytes;
				off_t num;

				ret = mpg123_decode_frame(m_handle, &num, &audio, &bytes);
				if (ret == MPG123_ERR)
					return false;
				if (ret == MPG123_NEW_FORMAT)
				{
					mpg123_getformat(m_handle, &m_sampleRate, &m_channels, &m_encoding);
					if (m_encoding != MPG123_ENC_FLOAT_32)
						return false;
				}

				if (m_channels > 0)
				{
					int32_t sampleCount = bytes / (m_channels * sizeof(float));
					int32_t channels = std::min(m_channels, 2);
					for (int32_t i = 0; i < sampleCount * m_channels; i += m_channels)
					{
						for (int32_t j = 0; j < channels; ++j)
						{
							float s = ((const float*)audio)[i + j];
							m_decoded[j][m_decodedCount] = s;
						}
						if (++m_decodedCount >= DecodedBufferSize)
							break;
					}
				}
			}
		}

		outSoundBlock.samples[SbcLeft] = m_decoded[SbcLeft];
		outSoundBlock.samples[SbcRight] = m_decoded[SbcRight];
		outSoundBlock.samplesCount = alignDown(std::min(m_decodedCount, outSoundBlock.samplesCount), 4);
		outSoundBlock.sampleRate = m_sampleRate;
		outSoundBlock.maxChannel = m_channels;

		m_consumedCount = outSoundBlock.samplesCount;

		if (!outSoundBlock.samplesCount || !outSoundBlock.sampleRate)
			return false;

		return true;
	}

private:
	static int32_t ms_instances;
	Ref< IStream > m_stream;
	mpg123_handle* m_handle;
	uint8_t m_readBuffer[ReadBufferSize];
	float* m_decoded[2];
	uint32_t m_decodedCount;
	uint32_t m_consumedCount;
	long m_sampleRate;
	int32_t m_channels;
	int32_t m_encoding;
};

int32_t Mp3StreamDecoderImpl::ms_instances = 0;

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
	m_decoderImpl->reset();
}

	}
}
