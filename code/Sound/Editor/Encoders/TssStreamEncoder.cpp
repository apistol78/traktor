/*
 * TRAKTOR
 * Copyright (c) 2022-203 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Math/MathUtils.h"
#include "Sound/Editor/Encoders/TssStreamEncoder.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.TssStreamEncoder", TssStreamEncoder, IStreamEncoder)

bool TssStreamEncoder::create(IStream* stream)
{
	m_stream = stream;
	return true;
}

void TssStreamEncoder::destroy()
{
	m_stream = nullptr;
}

bool TssStreamEncoder::putBlock(AudioBlock& block)
{
	Writer wr(m_stream);

	for (uint32_t offset = 0; offset < block.samplesCount; offset += 65535)
	{
		const uint32_t samplesCount = min< uint32_t >(block.samplesCount - offset, 65535);

		wr << samplesCount;
		wr << block.sampleRate;
		wr << block.maxChannel;

		uint16_t channelFlags = 0;
		for (uint32_t i = 0; i < block.maxChannel; ++i)
		{
			if (block.samples[i])
				channelFlags |= 1 << i;
		}
		wr << channelFlags;

		for (uint32_t i = 0; i < block.maxChannel; ++i)
		{
			if (block.samples[i])
				wr.write(block.samples[i] + offset, samplesCount, sizeof(float));
		}
	}

	return true;
}

}
