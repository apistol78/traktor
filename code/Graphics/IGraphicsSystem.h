/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_graphics_IGraphicsSystem_H
#define traktor_graphics_IGraphicsSystem_H

#include "Core/Object.h"
#include "Core/Platform.h"
#include "Graphics/PixelFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_GRAPHICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class ISurface;
struct SurfaceDesc;

/*! \brief Display mode description.
 * \ingroup Graphics
 */
struct DisplayMode
{
	int width;
	int height;
	uint8_t bits;
};

/*! \brief Graphics system create description.
 * \ingroup Graphics
 */
struct CreateDesc
{
	SystemWindow syswin;
	bool fullScreen;
	DisplayMode displayMode;
	PixelFormatEnum pixelFormat;	//< Secondary surface pixel format, primary surface has unknown pixel format.
};

/*! \brief Graphics system interface.
 * \ingroup Graphics
 */
class T_DLLCLASS IGraphicsSystem : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getDisplayModes(std::vector< DisplayMode >& outDisplayModes) const = 0;

	virtual bool getCurrentDisplayMode(DisplayMode& outDisplayMode) const = 0;

	virtual bool create(const CreateDesc& createDesc) = 0;

	virtual void destroy() = 0;

	virtual bool resize(int width, int height) = 0;

	virtual Ref< ISurface > getPrimarySurface() = 0;

	virtual Ref< ISurface > getSecondarySurface() = 0;

	virtual Ref< ISurface > createOffScreenSurface(const SurfaceDesc& surfaceDesc) = 0;

	virtual void flip(bool waitVBlank) = 0;
};

	}
}

#endif	// traktor_graphics_IGraphicsSystem_H
