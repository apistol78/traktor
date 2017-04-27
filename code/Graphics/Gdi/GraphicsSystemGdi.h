/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_graphics_GraphicsSystemGdi_H
#define traktor_graphics_GraphicsSystemGdi_H

#include <windows.h>
#include "Graphics/IGraphicsSystem.h"

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

class SurfaceGdi;

/*! \brief GDI graphics system implementation.
 * \ingroup GDI
 */
class T_DLLCLASS GraphicsSystemGdi : public IGraphicsSystem
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

	void flip(HDC hDC);

	HWND getWindow() const { return m_hWnd; }

	HDC getDeviceContext() const { return m_hDC; }

private:
	HWND m_hWnd;
	HDC m_hDC;
	uint32_t m_bitmapbuffer[sizeof(BITMAPINFO) + 16];
	BITMAPINFO* m_bitmapHeader;
	PixelFormatEnum m_secondaryFormat;
	Ref< SurfaceGdi > m_primarySurface;
	Ref< SurfaceGdi > m_secondarySurface;
};

	}
}

#endif	// traktor_graphics_GraphicsSystemGdi_H
