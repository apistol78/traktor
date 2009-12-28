#ifndef traktor_graphics_SurfaceDdWm5_H
#define traktor_graphics_SurfaceDdWm5_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "Core/Misc/ComRef.h"
#include "Graphics/ISurface.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_DDWM5_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class T_DLLCLASS SurfaceDdWm5 : public ISurface
{
	T_RTTI_CLASS;

public:
	SurfaceDdWm5(IDirectDrawSurface* dds);

	virtual bool getSurfaceDesc(SurfaceDesc& surfaceDesc) const;

	virtual void* lock(SurfaceDesc& surfaceDesc);

	virtual void unlock();

	virtual void blt(
		ISurface* sourceSurface,
		int sourceX,
		int sourceY,
		int x,
		int y,
		int width,
		int height
	);

	void swap(IDirectDrawSurface* dds);

private:
	ComRef< IDirectDrawSurface > m_dds;
	bool m_locked;
};

	}
}

#endif	// traktor_graphics_SurfaceDdWm5_H
