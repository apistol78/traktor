/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstdio>
#include <cstring>
#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Decoders/Mp3StreamDecoder.h"

#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

namespace traktor::sound
{

class Mp3StreamDecoderImpl : public Object
{
public:
	static size_t callbackRead(void* buf, size_t size, void* userData)
	{
		auto this_ = (Mp3StreamDecoderImpl*)userData;
		return this_->m_stream->read(buf, size);
	}

	static int callbackSeek(uint64_t position, void* userData)
	{
		auto this_ = (Mp3StreamDecoderImpl*)userData;
		return this_->m_stream->seek(IStream::SeekSet, position) < 0 ? 1 : 0;
	}

	bool create(IStream* stream)
	{
		m_stream = new BufferedStream(stream);

		m_io.read = &callbackRead;
		m_io.read_data = this;
		m_io.seek = &callbackSeek;
		m_io.seek_data = this;

		const int result = mp3dec_ex_open_cb(&m_dec, &m_io, MP3D_SEEK_TO_BYTE);
		if (result < 0)
		{
			log::error << L"MP3 decoder failed to open, error " << result << Endl;
			return false;
		}

		return true;
	}

	void destroy()
	{
		mp3dec_ex_close(&m_dec);
	}

	void reset()
	{
		mp3dec_ex_seek(&m_dec, 0);
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(AudioBlock& outBlock)
	{
		T_FATAL_ASSERT(outBlock.samplesCount * m_dec.info.channels <= sizeof_array(m_buffer));

		const size_t samplesRead = mp3dec_ex_read(&m_dec, m_buffer, outBlock.samplesCount * m_dec.info.channels);
		if (samplesRead == 0)
			return false;

		outBlock.maxChannel = m_dec.info.channels;
		outBlock.sampleRate = m_dec.info.hz;
		outBlock.samplesCount = samplesRead / m_dec.info.channels;

		for (int32_t i = 0; i < outBlock.maxChannel; ++i)
		{
			for (int32_t j = 0; j < outBlock.samplesCount; ++j)
			{
				const float s = m_buffer[j * outBlock.maxChannel + i];
				m_samples[i][j] = s;
			}
			outBlock.samples[i] = m_samples[i];
		}

		return true;
	}

private:
	Ref< IStream > m_stream;
	mp3dec_io_t m_io;
	mp3dec_ex_t m_dec;
	mp3d_sample_t m_buffer[2 * 4096];
	float m_samples[2][4096];
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Mp3StreamDecoder", 0, Mp3StreamDecoder, IStreamDecoder)

bool Mp3StreamDecoder::create(IStream* stream)
{
	if ((m_stream = stream) == nullptr)
		return false;

	m_decoderImpl = new Mp3StreamDecoderImpl();
	if (!m_decoderImpl->create(m_stream))
	{
		m_decoderImpl = nullptr;
		m_stream = nullptr;
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
	T_ASSERT(m_decoderImpl);
	return m_decoderImpl->getDuration();
}

bool Mp3StreamDecoder::getBlock(AudioBlock& outBlock)
{
	T_ASSERT(m_decoderImpl);
	return m_decoderImpl->getBlock(outBlock);
}

void Mp3StreamDecoder::rewind()
{
	T_ASSERT(m_decoderImpl);
	m_decoderImpl->reset();
}

}
