/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Drawing/Config.h"
#include "Drawing/IImageFormat.h"
#include "Drawing/Formats/ImageFormatBmp.h"
#include "Drawing/Formats/ImageFormatDds.h"
#if defined(DRAWING_INCLUDE_ICO)
#	include "Drawing/Formats/ImageFormatIco.h"
#endif
#if defined(DRAWING_INCLUDE_PNG)
#	include "Drawing/Formats/ImageFormatPng.h"
#endif
#if defined(DRAWING_INCLUDE_JPEG)
#	include "Drawing/Formats/ImageFormatJpeg.h"
#endif
#if defined(DRAWING_INCLUDE_GIF)
#	include "Drawing/Formats/ImageFormatGif.h"
#endif
#include "Drawing/Formats/ImageFormatHdr.h"
#if defined(DRAWING_INCLUDE_EXR)
#	include "Drawing/Formats/ImageFormatExr.h"
#endif
#include "Drawing/Formats/ImageFormatPcx.h"
#include "Drawing/Formats/ImageFormatTga.h"
#include "Drawing/Formats/ImageFormatTri.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.IImageFormat", IImageFormat, Object)

Ref< IImageFormat > IImageFormat::determineFormat(const std::wstring& extension)
{
	Ref< IImageFormat > imageFormat;

	if (compareIgnoreCase(extension, L"bmp") == 0)
		imageFormat = new ImageFormatBmp();
	else if (compareIgnoreCase(extension, L"dds") == 0)
		imageFormat = new ImageFormatDds();
#if defined(DRAWING_INCLUDE_ICO)
	else if (compareIgnoreCase(extension, L"ico") == 0)
		imageFormat = new ImageFormatIco();
#endif
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
	else if (compareIgnoreCase(extension, L"hdr") == 0)
		imageFormat = new ImageFormatHdr();
#if defined(DRAWING_INCLUDE_EXR)
	else if (compareIgnoreCase(extension, L"exr") == 0)
		imageFormat = new ImageFormatExr();
#endif
	else if (compareIgnoreCase(extension, L"pcx") == 0)
		imageFormat = new ImageFormatPcx();
	else if (compareIgnoreCase(extension, L"tga") == 0)
		imageFormat = new ImageFormatTga();
	else if (compareIgnoreCase(extension, L"tri") == 0)
		imageFormat = new ImageFormatTri();

	return imageFormat;
}

Ref< IImageFormat > IImageFormat::determineFormat(const Path& fileName)
{
	return determineFormat(fileName.getExtension());
}

}
