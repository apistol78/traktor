/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief GDI surface implementation.
 * \ingroup GDI
 */
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
	AlignedVector< uint8_t > m_data;
};

	}
}

#endif	// traktor_graphics_SurfaceGdi_H
