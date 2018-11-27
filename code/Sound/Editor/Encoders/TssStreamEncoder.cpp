/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Sound/Editor/Encoders/TssStreamEncoder.h"

namespace traktor
{
	namespace sound
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

bool TssStreamEncoder::putBlock(SoundBlock& block)
{
	Writer wr(m_stream);

	wr << block.samplesCount;
	wr << block.sampleRate;
	wr << block.maxChannel;

	for (uint32_t i = 0; i < block.maxChannel; ++i)
	{
		if (block.samples[i])
		{
			wr << uint8_t(0xff);
			wr.write(block.samples[i], block.samplesCount, sizeof(float));
		}
		else
			wr << uint8_t(0x00);
	}

	return true;
}

	}
}
