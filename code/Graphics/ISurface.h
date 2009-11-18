#ifndef traktor_graphics_ISurface_H
#define traktor_graphics_ISurface_H

#include "Core/Object.h"
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

struct SurfaceDesc
{
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	PixelFormatEnum pixelFormat;
};

class T_DLLCLASS ISurface : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getSurfaceDesc(SurfaceDesc& surfaceDesc) const = 0;

	virtual void* lock(SurfaceDesc& surfaceDesc) = 0;

	virtual void unlock() = 0;

	virtual void blt(
		ISurface* sourceSurface,
		int sourceX,
		int sourceY,
		int x,
		int y,
		int width,
		int height
	) = 0;
};

	}
}

#endif	// traktor_graphics_ISurface_H
