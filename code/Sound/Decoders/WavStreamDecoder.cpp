#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/Endian.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Decoders/WavStreamDecoder.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

#pragma pack(1)

struct RiffChunk
{
	uint8_t id[4];
	uint32_t size;
};

#pragma pack()

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.WavStreamDecoder", 0, WavStreamDecoder, IStreamDecoder)

bool WavStreamDecoder::create(IStream* stream)
{
	m_stream = stream;
	return readHeader();
}

void WavStreamDecoder::destroy()
{
	m_stream->close();
	m_stream = 0;
}

double WavStreamDecoder::getDuration() const
{
	return 0.0;
}

bool WavStreamDecoder::getBlock(SoundBlock& outSoundBlock)
{
	uint8_t block[32768 * 2];

	uint32_t requestBlockSize = outSoundBlock.samplesCount * (m_format.channels * m_format.bitsPerSample / 8);
	T_ASSERT (requestBlockSize <= sizeof(block));

	uint32_t readBlockSize = m_stream->read(block, requestBlockSize);
	if (!readBlockSize)
		return false;

	uint32_t readSamples = readBlockSize / (m_format.channels * m_format.bitsPerSample / 8);

	for (uint32_t channel = 0; channel < m_format.channels; ++channel)
	{
		float* samplesBuffer = m_samplesBuffers[channel];

		switch (m_format.bitsPerSample)
		{
		case 8:
			{
				uint8_t* blockPtr = &block[channel];
				for (uint32_t i = 0; i < readSamples; ++i)
				{
					samplesBuffer[i] = (*blockPtr / 255.0f) * 2.0f - 1.0f;
					blockPtr += m_format.channels;
				}
			}
			break;

		case 16:
			{
				uint16_t* blockPtr = reinterpret_cast< uint16_t* >(block) + channel;
				for (uint32_t i = 0; i < readSamples; ++i)
				{
#if defined(T_BIG_ENDIAN)
					swap8in32(*blockPtr);
#endif
					samplesBuffer[i] = *reinterpret_cast< int16_t* >(blockPtr) / 32767.0f;
					blockPtr += m_format.channels;
				}
			}
			break;
		}

		for (uint32_t i = readSamples; i < outSoundBlock.samplesCount; ++i)
			samplesBuffer[i] = 0.0f;

		outSoundBlock.samples[channel] = samplesBuffer;
	}

	outSoundBlock.samplesCount = alignUp(readSamples, 4);
	outSoundBlock.sampleRate = m_format.sampleRate;
	outSoundBlock.maxChannel = m_format.channels;

	return true;
}

void WavStreamDecoder::rewind()
{
	m_stream->seek(IStream::SeekSet, 0);
	readHeader();
}

bool WavStreamDecoder::readHeader()
{
	RiffChunk hdr, fmt, data;
	
	// Read RIFF header.
	m_stream->read(&hdr, sizeof(hdr));
	m_stream->seek(IStream::SeekCurrent, 4);
#if defined(T_BIG_ENDIAN)
	swap8in32(hdr.size);
#endif

	// Read format chunk.
	m_stream->read(&fmt, sizeof(fmt));
#if defined(T_BIG_ENDIAN)
	swap8in32(fmt.size);
#endif
	m_stream->read(&m_format, sizeof(m_format));
#if defined(T_BIG_ENDIAN)
	swap8in32(m_format.compression);
	swap8in32(m_format.channels);
	swap8in32(m_format.sampleRate);
	swap8in32(m_format.averageBytesPerSecond);
	swap8in32(m_format.blockAlign);
	swap8in32(m_format.bitsPerSample);
#endif
	m_stream->seek(IStream::SeekCurrent, fmt.size - sizeof(m_format));

	// Locate data chunk.
	for (;;)
	{
		if (m_stream->read(&data, sizeof(data)) != sizeof(data))
			return false;
		if (std::memcmp(data.id, "data", 4) == 0)
			break;
#if defined(T_BIG_ENDIAN)
		swap8in32(data.size);
#endif
		m_stream->seek(IStream::SeekCurrent, data.size);
	}

	return true;
}

	}
}
