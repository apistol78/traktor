/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Graphics/PixelFormat.h"

namespace traktor
{
	namespace graphics
	{

int getByteSize(PixelFormatEnum pixelFormat)
{
	const int c_byteSize[] = { 0, 2, 2, 2, 3, 4 };
	return c_byteSize[int(pixelFormat)];
}

int getColorBits(PixelFormatEnum pixelFormat)
{
	const int c_colorBits[] = { 0, 15, 15, 16, 24, 32 };
	return c_colorBits[int(pixelFormat)];
}

int getAlphaBits(PixelFormatEnum pixelFormat)
{
	const int c_alphaBits[] = { 0, 0, 1, 0, 0, 8 };
	return c_alphaBits[int(pixelFormat)];
}

uint32_t getRedMask(PixelFormatEnum pixelFormat)
{
	uint32_t c_redMasks[] =
	{
		0x00000000,
		0x00007C00,		// PfeR5G5B5
		0x00007C00,		// PfeA1R5G5B5
		0x0000F800,		// PfeR5G6B5
		0x00FF0000,		// PfeR8G8B8
		0x00FF0000		// PfeA8R8G8B8
	};
	return c_redMasks[int(pixelFormat)];
}

uint32_t getGreenMask(PixelFormatEnum pixelFormat)
{
	uint32_t c_greenMasks[] =
	{
		0x00000000,
		0x000003E0,
		0x000003E0,
		0x000007E0,
		0x0000FF00,
		0x0000FF00
	};
	return c_greenMasks[int(pixelFormat)];
}

uint32_t getBlueMask(PixelFormatEnum pixelFormat)
{
	uint32_t c_blueMasks[] =
	{
		0x00000000,
		0x0000001F,
		0x0000001F,
		0x0000001F,
		0x000000FF,
		0x000000FF
	};
	return c_blueMasks[int(pixelFormat)];
}

bool hasAlpha(PixelFormatEnum pixelFormat)
{
	const bool c_hasAlpha[] = { false, false, true, false, false, true };
	return c_hasAlpha[int(pixelFormat)];
}

void T_DLLCLASS convertPixel(
	PixelFormatEnum targetPixelFormat,
	void* targetPixel,
	PixelFormatEnum sourcePixelFormat,
	const void* sourcePixel
)
{
	const uint8_t* sp = static_cast< const uint8_t* >(sourcePixel);
	uint8_t* tp = static_cast< uint8_t* >(targetPixel);
	uint8_t rgb[3] = { 0, 0, 0 };

	switch (sourcePixelFormat)
	{
	case PfeR5G5B5:
		break;

	case PfeA1R5G5B5:
		break;

	case PfeR5G6B5:
		break;

	case PfeR8G8B8:
	case PfeA8R8G8B8:
		rgb[0] = sp[2];
		rgb[1] = sp[1];
		rgb[2] = sp[0];
		break;
		
	default:
		break;
	}

	switch (targetPixelFormat)
	{
	case PfeR5G5B5:
		break;

	case PfeA1R5G5B5:
		break;

	case PfeR5G6B5:
		break;

	case PfeR8G8B8:
	case PfeA8R8G8B8:
		tp[2] = rgb[0];
		tp[1] = rgb[1];
		tp[0] = rgb[2];
		break;

	default:
		break;
	}
}

	}
}
