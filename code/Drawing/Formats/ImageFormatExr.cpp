/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Config.h"
#if defined(DRAWING_INCLUDE_EXR)

#include <cstring>
#include <tinyexr.h>

#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Drawing/Formats/ImageFormatExr.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatExr", ImageFormatExr, IImageFormat)

Ref< Image > ImageFormatExr::read(IStream* stream)
{
	AlignedVector< uint8_t > memory;

	// Load entire file into memory.
	DynamicMemoryStream ms(memory, false, true);
	if (!StreamCopy(&ms, stream).execute())
		return nullptr;

	// Parse image data.
	float* rgba = nullptr;
	int width = 0;
	int height = 0;
	if (LoadEXRFromMemory(&rgba, &width, &height, memory.c_ptr(), memory.size(), nullptr) < 0)
		return nullptr;

	memory.clear();

	// Create image.
	Ref< drawing::Image > image = new drawing::Image(
		PixelFormat::getRGBAF32(),
		width,
		height
	);

	Ref< ImageInfo > imageInfo = new ImageInfo();
	imageInfo->setGamma(1.0f);
	image->setImageInfo(imageInfo);

	std::memcpy(image->getData(), rgba, width * height * (4 * sizeof(float)));
	std::free(rgba);
	return image;
}

bool ImageFormatExr::write(IStream* stream, const Image* image)
{
	Ref< Image > img = image->clone();
	img->convert(PixelFormat::getRGBAF32());

	if (image->getPixelFormat().getAlphaBits() == 0)
		img->clearAlpha(1.0f);

	uint8_t* buffer = nullptr;
	const int32_t size = SaveEXRToMemory(
		(const float*)img->getData(),
		img->getWidth(),
		img->getHeight(),
		4,
		0,
		(unsigned char**)&buffer,
		nullptr
	);
	if (size <= 0)
		return false;

	MemoryStream ms(buffer, size, true, false);
	const bool result = StreamCopy(stream, &ms).execute();

	std::free(buffer);
	return result;
}

}

#endif	// DRAWING_INCLUDE_EXR
