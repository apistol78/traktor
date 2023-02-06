/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstdio>
#include <cstring>
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Drawing/Formats/ImageFormatHdr.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"

namespace traktor::drawing
{
	namespace
	{

#pragma pack(1)
struct RGBE
{
	union
	{
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t e;
		};
		uint8_t ch[4];
	};
};
#pragma pack()

#define MINELEN 8
#define MAXELEN 0x7fff

float convertComponent(int32_t expo, int32_t val)
{
	if (expo == -128)
		return 0.0f;
	float v = val / 256.0f;
	float d = std::pow(2.0f, expo);
	return v * d;
}

void workOnRGBE(const RGBE* scan, float* data, int32_t len)
{
	while (len-- > 0)
	{
		int32_t expo = (int32_t)scan->e - 128;
		data[0] = convertComponent(expo, scan->r);
		data[1] = convertComponent(expo, scan->g);
		data[2] = convertComponent(expo, scan->b);
		data[3] = 1.0f;
		data += 4;
		scan++;
	}
}

bool readByte(IStream* stream, uint8_t& outByte)
{
	return stream->read(&outByte, 1) == 1;
}

int32_t readByte(IStream* stream)
{
	uint8_t b;
	if (readByte(stream, b))
		return (int32_t)b;
	else
		return -1;
}

bool decrunchLegacy(IStream* stream, RGBE* scan, int32_t len)
{
	int32_t i;
	int32_t rshift = 0;

	while (len > 0)
	{
		if (stream->read(scan[0].ch, 4) != 4)
			return false;

		if (scan[0].r == 1 && scan[0].g == 1 && scan[0].b == 1)
		{
			for (i = scan[0].e << rshift; i > 0; i--)
			{
				std::memcpy(&scan[0], &scan[-1], 4);
				scan++;
				len--;
			}
			rshift += 8;
		}
		else
		{
			scan++;
			len--;
			rshift = 0;
		}
	}
	return true;
}

bool decrunch(IStream* stream, RGBE* scan, int32_t len)
{
	int32_t i, j;

	if (len < MINELEN || len > MAXELEN)
		return decrunchLegacy(stream, scan, len);

	if ((i = readByte(stream)) < 0)
		return false;
	if (i != 2)
	{
		stream->seek(IStream::SeekCurrent, -1);
		return decrunchLegacy(stream, scan, len);
	}

	readByte(stream, scan[0].g);
	readByte(stream, scan[0].b);
	if ((i = readByte(stream)) < 0)
		return false;

	if (scan[0].g != 2 || scan[0].b & 128)
	{
		scan[0].r = 2;
		scan[0].e = i;
		return decrunchLegacy(stream, scan + 1, len - 1);
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < len; )
		{
			uint8_t code;
			if (!readByte(stream, code))
				return false;
			if (code > 128)
			{
				code &= 127;
				uint8_t val;
				if (!readByte(stream, val))
					return false;
				while (code--)
					scan[j++].ch[i] = val;
			}
			else 
			{
				while(code--)
				{
					if (!readByte(stream, scan[j++].ch[i]))
						return false;
				}
			}
		}
	}

	return true;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatHdr", ImageFormatHdr, IImageFormat)

Ref< Image > ImageFormatHdr::read(IStream* stream)
{
	char str[200] = { 0 };

	if (stream->read(str, 10) != 10)
		return nullptr;
	if (std::memcmp(str, "#?RADIANCE", 10) != 0)
	{
		stream->seek(IStream::SeekSet, 0);
		stream->read(str, 6);
		if (std::memcmp(str, "#?RGBE", 6) != 0)
			return nullptr;
	}

	stream->seek(IStream::SeekCurrent, 1);

	char c = 0, oldc;
	for (;;)
	{
		oldc = c;
		if (stream->read(&c, 1) != 1)
			return nullptr;
		if (c == 0xa && oldc == 0xa)
			break;
	}

	char reso[200] = { 0 };
	int32_t i = 0;
	for (;;)
	{
		if (stream->read(&c, 1) != 1)
			return nullptr;
		reso[i++] = c;
		if (c == 0xa)
			break;
	}

	int32_t w, h;
	if (!std::sscanf(reso, "-Y %d +X %d", &h, &w))
		return nullptr;

	Ref< Image > image = new Image(PixelFormat::getRGBAF32(), w, h);
	
	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setGamma(1.0f);
	image->setImageInfo(imageInfo);

	AutoArrayPtr< RGBE > scan(new RGBE[w]);
	if (!scan.c_ptr())
		return nullptr;

	float* data = (float*)image->getData();
	for (int32_t y = h - 1; y >= 0; --y)
	{
		if (decrunch(stream, scan.ptr(), w) == false)
			break;
		workOnRGBE(scan.c_ptr(), data, w);
		data += w * 4;
	}

	return image;
}

bool ImageFormatHdr::write(IStream* stream, const Image* image)
{
	return false;
}

}
