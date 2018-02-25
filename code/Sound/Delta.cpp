/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Sound/Delta.h"

namespace traktor
{
	namespace sound
	{

uint32_t Delta::encode(const int16_t* data, uint32_t count, BitWriter& bw)
{
	uint32_t nibbles = 4;
	bw.writeUnsigned(16, data[0] + 32767);
	for (uint32_t i = 1; i < count; ++i)
	{
		int32_t delta = data[i] - data[i - 1];
		if (delta >= -7 && delta <= 7)
		{
			bw.writeUnsigned(4, delta + 7);
			nibbles++;
		}
		else
		{
			bw.writeUnsigned(4, 15);
			if (delta >= -127 && delta <= 127)
				bw.writeUnsigned(8, delta + 127);
			else
			{
				bw.writeUnsigned(8, 255);
				bw.writeUnsigned(16, delta + 32767);
				nibbles += 4;
			}
			nibbles += 3;
		}
	}
	return nibbles / 2;
}

void Delta::decode(BitReader& br, uint32_t count, int16_t* out)
{
	uint32_t i = 0;

	int16_t sample = br.readUnsigned(16) - 32767;
	out[i++] = sample;

	while (i < count)
	{
		int16_t delta;
		int32_t n;

		n = br.readUnsigned(4);
		if (n != 15)
		{
			delta = n - 7;
		}
		else
		{
			n = br.readUnsigned(8);
			if (n != 255)
			{
				delta = n - 127;
			}
			else
			{
				n = br.readUnsigned(16);
				delta = n - 32767;
			}
		}

		sample += delta;

		out[i++] = sample;
	}
}

	}
}
