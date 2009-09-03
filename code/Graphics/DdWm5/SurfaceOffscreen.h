#ifndef traktor_graphics_SurfaceOffscreen_H
#define traktor_graphics_SurfaceOffscreen_H

#include "Graphics/Surface.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_GAPI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class T_DLLCLASS SurfaceOffscreen : public Surface
{
	T_RTTI_CLASS(SurfaceOffscreen)

public:
	SurfaceOffscreen(const SurfaceDesc& surfaceDesc);

	virtual bool getSurfaceDesc(SurfaceDesc& surfaceDesc) const;

	virtual void* lock(SurfaceDesc& surfaceDesc);

	virtual void unlock();

	virtual void blt(
		Surface* sourceSurface,
		int sourceX,
		int sourceY,
		int x,
		int y,
		int width,
		int height
	);

private:
	SurfaceDesc m_surfaceDesc;
	AutoArrayPtr< uint8_t > m_data;
};

	}
}

#endif	// traktor_graphics_SurfaceOffscreen_H
