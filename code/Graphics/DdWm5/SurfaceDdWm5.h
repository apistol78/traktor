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

	virtual ~SurfaceDdWm5();

	virtual bool getSurfaceDesc(SurfaceDesc& surfaceDesc) const;

	virtual void* lock(SurfaceDesc& surfaceDesc);

	virtual void unlock();

	virtual void blt(
		ISurface* sourceSurface,
		int32_t sourceX,
		int32_t sourceY,
		int32_t x,
		int32_t y,
		int32_t width,
		int32_t height
	);

	virtual void setFontSize(int32_t size);

	virtual void getTextExtent(const std::wstring& text, int32_t& outWidth, int32_t& outHeight);

	virtual void drawText(
		int32_t x,
		int32_t y,
		const std::wstring& text,
		const Color4ub& color
	);

	void swap(IDirectDrawSurface* dds);

private:
	ComRef< IDirectDrawSurface > m_dds;
	HFONT m_hFont;
	bool m_locked;
};

	}
}

#endif	// traktor_graphics_SurfaceDdWm5_H
