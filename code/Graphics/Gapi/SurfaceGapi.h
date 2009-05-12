#ifndef traktor_graphics_SurfaceGapi_H
#define traktor_graphics_SurfaceGapi_H

#include <vector>
#include "Graphics/Surface.h"

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

class T_DLLCLASS SurfaceGapi : public Surface
{
	T_RTTI_CLASS(SurfaceGapi)

public:
	SurfaceGapi(const SurfaceDesc& surfaceDesc);

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
	std::vector< unsigned char > m_data;
};

	}
}

#endif	// traktor_graphics_SurfaceGapi_H
