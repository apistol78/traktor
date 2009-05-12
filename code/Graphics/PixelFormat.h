#ifndef traktor_graphics_PixelFormat_H
#define traktor_graphics_PixelFormat_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

enum PixelFormatEnum
{
	PfeUnknown  = 0,
	PfeR5G5B5	= 1,
	PfeA1R5G5B5	= 2,
	PfeR5G6B5	= 3,
	PfeR8G8B8	= 4,
	PfeA8R8G8B8	= 5
};

int T_DLLCLASS getByteSize(PixelFormatEnum pixelFormat);

int T_DLLCLASS getColorBits(PixelFormatEnum pixelFormat);

int T_DLLCLASS getAlphaBits(PixelFormatEnum pixelFormat);

bool T_DLLCLASS hasAlpha(PixelFormatEnum pixelFormat);

void T_DLLCLASS convertPixel(
	PixelFormatEnum targetPixelFormat,
	void* targetPixel,
	PixelFormatEnum sourcePixelFormat,
	const void* sourcePixel
);

	}
}

#endif	// traktor_graphics_PixelFormat_H
