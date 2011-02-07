#ifndef traktor_graphics_SurfaceGdi_H
#define traktor_graphics_SurfaceGdi_H

#include "Core/Containers/AlignedVector.h"
#include "Graphics/ISurface.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_GDI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class T_DLLCLASS SurfaceGdi : public ISurface
{
	T_RTTI_CLASS;

public:
	SurfaceGdi(const SurfaceDesc& surfaceDesc);

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

private:
	SurfaceDesc m_surfaceDesc;
	AlignedVector< uint8_t > m_data;
};

	}
}

#endif	// traktor_graphics_SurfaceGdi_H
