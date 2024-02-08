/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include <FLAC/stream_decoder.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Memory/Alloc.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Decoders/FlacStreamDecoder.h"

namespace traktor::sound
{

class FlacStreamDecoderImpl : public Object
{
public:
	FlacStreamDecoderImpl()
	:	m_decoder(nullptr)
	,	m_decodedCount(0)
	,	m_keepOffset(0)
	{
	}

	virtual ~FlacStreamDecoderImpl()
	{
		T_ASSERT(!m_decoder);
	}

	bool create(IStream* stream)
	{
		FLAC__StreamDecoderInitStatus status;

		m_stream = stream;

		m_decoded[0] = (float*)Alloc::acquireAlign(sizeof(float) * 65536, 16, T_FILE_LINE);
		m_decoded[1] = (float*)Alloc::acquireAlign(sizeof(float) * 65536, 16, T_FILE_LINE);

		m_decoder = FLAC__stream_decoder_new();
		if (!m_decoder)
			return false;

		FLAC__stream_decoder_set_md5_checking(m_decoder, false);

		status = FLAC__stream_decoder_init_stream(
			m_decoder,
			&FlacStreamDecoderImpl::readCallback,
			0,
			0,
			0,
			0,
			&FlacStreamDecoderImpl::writeCallback,
			0,
			&FlacStreamDecoderImpl::errorCallback,
			(void*)this
		);
		if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		{
			destroy();
			return false;
		}

		return true;
	}

	void destroy()
	{
		FLAC__stream_decoder_delete(m_decoder);

		Alloc::freeAlign(m_decoded[0]);
		Alloc::freeAlign(m_decoded[1]);

		m_decoder = nullptr;
	}

	double getDuration() const
	{
		return 0.0;
	}

	bool getBlock(AudioBlock& outBlock)
	{
		m_keepOffset = std::min(m_keepOffset, m_decodedCount);

		if (m_keepOffset > 0)
		{
			std::memmove(&m_decoded[SbcLeft][0], &m_decoded[SbcLeft][m_keepOffset], (m_decodedCount - m_keepOffset) * sizeof(float));
			std::memmove(&m_decoded[SbcRight][0], &m_decoded[SbcRight][m_keepOffset], (m_decodedCount - m_keepOffset) * sizeof(float));
			m_decodedCount -= m_keepOffset;
		}

		while (m_decodedCount < outBlock.samplesCount)
		{
			FLAC__bool decodeResult = FLAC__stream_decoder_process_single(m_decoder);
			if (!decodeResult)
				return false;

			const auto state = FLAC__stream_decoder_get_state(m_decoder);
			if (state >= FLAC__STREAM_DECODER_END_OF_STREAM)
				break;
		}

		if (m_decodedCount == 0)
			return false;

		outBlock.samples[SbcLeft] = m_decoded[SbcLeft];
		outBlock.samples[SbcRight] = m_decoded[SbcRight];
		outBlock.samplesCount = std::min(m_decodedCount, outBlock.samplesCount);
		outBlock.sampleRate = FLAC__stream_decoder_get_sample_rate(m_decoder);
		outBlock.maxChannel = FLAC__stream_decoder_get_channels(m_decoder);

		m_keepOffset = outBlock.samplesCount;
		return true;
	}

private:
	Ref< IStream > m_stream;
	FLAC__StreamDecoder* m_decoder;
	float* m_decoded[2];
	uint32_t m_decodedCount;
	uint32_t m_keepOffset;

	static FLAC__StreamDecoderReadStatus readCallback(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* clientData)
	{
		FlacStreamDecoderImpl* this_ = reinterpret_cast< FlacStreamDecoderImpl* >(clientData);
		*bytes = this_->m_stream->read(buffer, int(*bytes));
		return *bytes ? FLAC__STREAM_DECODER_READ_STATUS_CONTINUE : FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}

	static FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* clientData)
	{
		FlacStreamDecoderImpl* this_ = reinterpret_cast< FlacStreamDecoderImpl* >(clientData);

		uint32_t bps = FLAC__stream_decoder_get_bits_per_sample(this_->m_decoder);
		uint32_t channels = FLAC__stream_decoder_get_channels(this_->m_decoder);

		switch (bps)
		{
		case 8:
			for (uint32_t i = 0; i < frame->header.blocksize; ++i)
			{
				this_->m_decoded[SbcLeft][this_->m_decodedCount] = float(buffer[0][i] / 128.0f);
				if (channels >= 2)
					this_->m_decoded[SbcRight][this_->m_decodedCount] = float(buffer[1][i] / 128.0f);
				this_->m_decodedCount++;
			}
			break;

		case 16:
			for (uint32_t i = 0; i < frame->header.blocksize; ++i)
			{
				this_->m_decoded[SbcLeft][this_->m_decodedCount] = float(buffer[0][i] / 32768.0f);
				if (channels >= 2)
					this_->m_decoded[SbcRight][this_->m_decodedCount] = float(buffer[1][i] / 32768.0f);
				this_->m_decodedCount++;
			}
			break;
		}

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}

	static void errorCallback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* clientData)
	{
		log::error << L"Sound - FLAC error \"" << mbstows(FLAC__StreamDecoderErrorStatusString[status]) << L"\"" << Endl;
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.FlacStreamDecoder", 0, FlacStreamDecoder, IStreamDecoder)

bool FlacStreamDecoder::create(IStream* stream)
{
	m_stream = stream;
	rewind();
	return m_decoderImpl != nullptr;
}

void FlacStreamDecoder::destroy()
{
	if (m_decoderImpl)
	{
		m_decoderImpl->destroy();
		m_decoderImpl = nullptr;
	}
}

double FlacStreamDecoder::getDuration() const
{
	T_ASSERT(m_decoderImpl);
	return m_decoderImpl->getDuration();
}

bool FlacStreamDecoder::getBlock(AudioBlock& outBlock)
{
	T_ASSERT(m_decoderImpl);
	return m_decoderImpl->getBlock(outBlock);
}

void FlacStreamDecoder::rewind()
{
	destroy();
	m_stream->seek(IStream::SeekSet, 0);
	m_decoderImpl = new FlacStreamDecoderImpl();
	if (!m_decoderImpl->create(m_stream))
		m_decoderImpl = nullptr;
}

}
