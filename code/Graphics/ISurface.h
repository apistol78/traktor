/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_graphics_ISurface_H
#define traktor_graphics_ISurface_H

#include "Core/Object.h"
#include "Core/Math/Color4ub.h"
#include "Graphics/PixelFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

/*! \brief Surface description.
 * \ingroup Graphics
 */
struct SurfaceDesc
{
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	PixelFormatEnum pixelFormat;
};

/*! \brief Surface interface.
 * \ingroup Graphics
 */
class T_DLLCLASS ISurface : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getSurfaceDesc(SurfaceDesc& surfaceDesc) const = 0;

	virtual void* lock(SurfaceDesc& surfaceDesc) = 0;

	virtual void unlock() = 0;

	virtual void blt(
		ISurface* sourceSurface,
		int32_t sourceX,
		int32_t sourceY,
		int32_t x,
		int32_t y,
		int32_t width,
		int32_t height
	) = 0;

	virtual void setFontSize(int32_t size) = 0;

	virtual void getTextExtent(const std::wstring& text, int32_t& outWidth, int32_t& outHeight) = 0;

	virtual void drawText(
		int32_t x,
		int32_t y,
		const std::wstring& text,
		const Color4ub& color
	) = 0;
};

	}
}

#endif	// traktor_graphics_ISurface_H
