#ifndef traktor_graphics_SurfaceDd7_H
#define traktor_graphics_SurfaceDd7_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "Graphics/Surface.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_DD7_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class T_DLLCLASS SurfaceDd7 : public Surface
{
	T_RTTI_CLASS(SurfaceDd7)

public:
	SurfaceDd7(IDirectDrawSurface7* dds);

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

	void swap(IDirectDrawSurface7* dds);

private:
	ComRef< IDirectDrawSurface7 > m_dds;
	bool m_locked;
};

	}
}

#endif	// traktor_graphics_SurfaceDd7_H
