/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Sound/Decoders/TssStreamDecoder.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.TssStreamDecoder", 0, TssStreamDecoder, IStreamDecoder)

TssStreamDecoder::TssStreamDecoder()
{
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		m_samplesBuffer[i] = nullptr;
}

bool TssStreamDecoder::create(IStream* stream)
{
	m_stream = stream;
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		m_samplesBuffer[i] = (float*)Alloc::acquireAlign(65535 * sizeof(float), 16, T_FILE_LINE);
		if (!m_samplesBuffer[i])
			return false;
	}
	return true;
}

void TssStreamDecoder::destroy()
{
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		if (m_samplesBuffer[i])
		{
			Alloc::freeAlign(m_samplesBuffer[i]);
			m_samplesBuffer[i] = nullptr;
		}
	}
	safeClose(m_stream);
}

double TssStreamDecoder::getDuration() const
{
	return 0.0;
}

bool TssStreamDecoder::getBlock(SoundBlock& outSoundBlock)
{
	if (m_stream->available() <= 0)
		return false;

	Reader rd(m_stream);
	uint8_t have;

	rd >> outSoundBlock.samplesCount;
	rd >> outSoundBlock.sampleRate;
	rd >> outSoundBlock.maxChannel;

	T_FATAL_ASSERT(outSoundBlock.samplesCount <= 65535);

	for (uint32_t i = 0; i < outSoundBlock.maxChannel; ++i)
	{
		rd >> have;
		if (have)
		{
			rd.read(m_samplesBuffer[i], outSoundBlock.samplesCount, sizeof(float));
			outSoundBlock.samples[i] = m_samplesBuffer[i];
		}
		else
			outSoundBlock.samples[i] = nullptr;
	}

	return true;
}

void TssStreamDecoder::rewind()
{
	m_stream->seek(IStream::SeekSet, 0);
}

	}
}
