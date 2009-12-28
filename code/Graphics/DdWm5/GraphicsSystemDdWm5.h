#ifndef traktor_graphics_GraphicsSystemDdWm5_H
#define traktor_graphics_GraphicsSystemDdWm5_H

#include <ddraw.h>
#include "Core/Misc/ComRef.h"
#include "Graphics/IGraphicsSystem.h"

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

class SurfaceDdWm5;

class T_DLLCLASS GraphicsSystemDdWm5 : public IGraphicsSystem
{
	T_RTTI_CLASS;

public:
	GraphicsSystemDdWm5();

	virtual bool getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const;

	virtual bool create(const CreateDesc& createDesc);

	virtual void destroy();

	virtual bool resize(int width, int height);

	virtual Ref< ISurface > getPrimarySurface();

	virtual Ref< ISurface > getSecondarySurface();

	virtual Ref< ISurface > createOffScreenSurface(const SurfaceDesc& surfaceDesc);

	virtual void flip(bool waitVBlank);

private:
	HWND m_hWnd;
	ComRef< IDirectDraw > m_dd;
	ComRef< IDirectDrawSurface > m_ddsPrimary;
	ComRef< IDirectDrawSurface > m_ddsSecondary;
	DDPIXELFORMAT m_ddPixelFormat;
	Ref< SurfaceDdWm5 > m_primary;
	Ref< SurfaceDdWm5 > m_secondary;
	bool m_supportHardwareFlip;
};

	}
}

#endif	// traktor_graphics_GraphicsSystemDdWm5_H
