/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_graphics_SurfaceDd7_H
#define traktor_graphics_SurfaceDd7_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "Graphics/ISurface.h"
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

class T_DLLCLASS SurfaceDd7 : public ISurface
{
	T_RTTI_CLASS;

public:
	SurfaceDd7(IDirectDrawSurface7* dds);

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

	void swap(IDirectDrawSurface7* dds);

private:
	ComRef< IDirectDrawSurface7 > m_dds;
	bool m_locked;
};

	}
}

#endif	// traktor_graphics_SurfaceDd7_H
