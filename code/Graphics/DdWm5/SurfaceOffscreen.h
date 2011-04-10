#ifndef traktor_graphics_SurfaceOffscreen_H
#define traktor_graphics_SurfaceOffscreen_H

#include "Core/Misc/AutoPtr.h"
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

class T_DLLCLASS SurfaceOffscreen : public ISurface
{
	T_RTTI_CLASS;

public:
	SurfaceOffscreen(const SurfaceDesc& surfaceDesc);

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

	virtual void setFontSize(int32_t size);

	virtual void getTextExtent(const std::wstring& text, int32_t& outWidth, int32_t& outHeight);

	virtual void drawText(
		int32_t x,
		int32_t y,
		const std::wstring& text,
		const Color4ub& color
	);

private:
	SurfaceDesc m_surfaceDesc;
	AutoArrayPtr< uint8_t > m_data;
};

	}
}

#endif	// traktor_graphics_SurfaceOffscreen_H
