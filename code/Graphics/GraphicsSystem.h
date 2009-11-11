#ifndef traktor_graphics_GraphicsSystem_H
#define traktor_graphics_GraphicsSystem_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Graphics/PixelFormat.h"

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

class Surface;
struct SurfaceDesc;

struct DisplayMode
{
	int width;
	int height;
	uint8_t bits;
};

struct CreateDesc
{
	void* windowHandle;
	bool fullScreen;
	DisplayMode displayMode;
	PixelFormatEnum pixelFormat;	//< Secondary surface pixel format, primary surface has unknown pixel format.
};

class T_DLLCLASS GraphicsSystem : public Object
{
	T_RTTI_CLASS(GraphicsSystem)

public:
	virtual bool getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const = 0;

	virtual bool create(const CreateDesc& createDesc) = 0;

	virtual void destroy() = 0;

	virtual bool resize(int width, int height) = 0;

	virtual Ref< Surface > getPrimarySurface() = 0;

	virtual Ref< Surface > getSecondarySurface() = 0;

	virtual Ref< Surface > createOffScreenSurface(const SurfaceDesc& surfaceDesc) = 0;

	virtual void flip(bool waitVBlank) = 0;
};

	}
}

#endif	// traktor_graphics_GraphicsSystem_H
