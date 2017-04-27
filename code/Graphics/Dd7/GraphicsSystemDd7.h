/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_graphics_GraphicsSystemDd7_H
#define traktor_graphics_GraphicsSystemDd7_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "Graphics/IGraphicsSystem.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_DD7_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class SurfaceDd7;

class T_DLLCLASS GraphicsSystemDd7 : public IGraphicsSystem
{
	T_RTTI_CLASS;

public:
	virtual bool getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const;

	virtual bool getCurrentDisplayMode(DisplayMode& outDisplayMode) const;

	virtual bool create(const CreateDesc& createDesc);

	virtual void destroy();

	virtual bool resize(int width, int height);

	virtual Ref< ISurface > getPrimarySurface();

	virtual Ref< ISurface > getSecondarySurface();

	virtual Ref< ISurface > createOffScreenSurface(const SurfaceDesc& surfaceDesc);

	virtual void flip(bool waitVBlank);

private:
	HWND m_hWnd;
	ComRef< IDirectDraw7 > m_dd;
	ComRef< IDirectDrawSurface7 > m_ddsPrimary;
	ComRef< IDirectDrawSurface7 > m_ddsSecondary;
	ComRef< IDirectDrawClipper > m_ddClipper;
	Ref< SurfaceDd7 > m_primary;
	Ref< SurfaceDd7 > m_secondary;
	DDPIXELFORMAT m_ddpf;
	bool m_bltFlip;
};

	}
}

#endif	// traktor_graphics_GraphicsSystemDd7_H
