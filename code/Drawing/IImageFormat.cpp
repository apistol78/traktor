#include "Drawing/Config.h"
#include "Drawing/IImageFormat.h"
#include "Drawing/Formats/ImageFormatBmp.h"
#if defined(DRAWING_INCLUDE_PNG)
#	include "Drawing/Formats/ImageFormatPng.h"
#endif
#if defined(DRAWING_INCLUDE_JPEG)
#	include "Drawing/Formats/ImageFormatJpeg.h"
#endif
#if defined(DRAWING_INCLUDE_GIF)
#	include "Drawing/Formats/ImageFormatGif.h"
#endif
#if defined(DRAWING_INCLUDE_EXR)
#	include "Drawing/Formats/ImageFormatExr.h"
#endif
#include "Drawing/Formats/ImageFormatTga.h"
#include "Drawing/Formats/ImageFormatPcx.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.IImageFormat", IImageFormat, Object)

Ref< IImageFormat > IImageFormat::determineFormat(const std::wstring& extension)
{
	Ref< IImageFormat > imageFormat;

	if (compareIgnoreCase(extension, L"bmp") == 0)
		imageFormat = new ImageFormatBmp();
#if defined(DRAWING_INCLUDE_PNG)
	else if (compareIgnoreCase(extension, L"png") == 0)
		imageFormat = new ImageFormatPng();
#endif
#if defined(DRAWING_INCLUDE_JPEG)
	else if (compareIgnoreCase(extension, L"jpg") == 0 || compareIgnoreCase(extension, L"jpeg") == 0)
		imageFormat = new ImageFormatJpeg();
#endif
#if defined(DRAWING_INCLUDE_GIF)
	else if (compareIgnoreCase(extension, L"gif") == 0)
		imageFormat = new ImageFormatGif();
#endif
#if defined(DRAWING_INCLUDE_EXR)
	else if (compareIgnoreCase(extension, L"exr") == 0)
		imageFormat = new ImageFormatExr();
#endif
	else if (compareIgnoreCase(extension, L"tga") == 0)
		imageFormat = new ImageFormatTga();
	else if (compareIgnoreCase(extension, L"pcx") == 0)
		imageFormat = new ImageFormatPcx();

	return imageFormat;
}

Ref< IImageFormat > IImageFormat::determineFormat(const Path& fileName)
{
	return determineFormat(fileName.getExtension());
}

	}
}
