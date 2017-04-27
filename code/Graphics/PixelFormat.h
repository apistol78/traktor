/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief Pixel formats.
 * \ingroup Graphics
 */
enum PixelFormatEnum
{
	PfeUnknown  = 0,
	PfeR5G5B5	= 1,
	PfeA1R5G5B5	= 2,
	PfeR5G6B5	= 3,
	PfeR8G8B8	= 4,
	PfeA8R8G8B8	= 5
};

/*! \brief Get size, in bytes, of pixel format.
 * \ingroup Graphics
 */
int T_DLLCLASS getByteSize(PixelFormatEnum pixelFormat);

/*! \brief Get number of color bits of pixel format.
 * \ingroup Graphics
 */
int T_DLLCLASS getColorBits(PixelFormatEnum pixelFormat);

/*! \brief Get number of alpha bits of pixel format.
 * \ingroup Graphics
 */
int T_DLLCLASS getAlphaBits(PixelFormatEnum pixelFormat);

/*! \brief Get red bit-mask of pixel format.
 * \ingroup Graphics
 */
uint32_t T_DLLCLASS getRedMask(PixelFormatEnum pixelFormat);

/*! \brief Get green bit-mask of pixel format.
 * \ingroup Graphics
 */
uint32_t T_DLLCLASS getGreenMask(PixelFormatEnum pixelFormat);

/*! \brief Get blue bit-mask of pixel format.
 * \ingroup Graphics
 */
uint32_t T_DLLCLASS getBlueMask(PixelFormatEnum pixelFormat);

/*! \brief Check if pixel format has alpha channel defined.
 * \ingroup Graphics
 */
bool T_DLLCLASS hasAlpha(PixelFormatEnum pixelFormat);

/*! \brief Convert pixel from one pixel format to another.
 * \ingroup Graphics
 */
void T_DLLCLASS convertPixel(
	PixelFormatEnum targetPixelFormat,
	void* targetPixel,
	PixelFormatEnum sourcePixelFormat,
	const void* sourcePixel
);

	}
}

#endif	// traktor_graphics_PixelFormat_H
