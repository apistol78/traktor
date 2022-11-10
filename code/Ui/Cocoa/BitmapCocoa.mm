/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Color4ub.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Cocoa/BitmapCocoa.h"

namespace traktor
{
	namespace ui
	{

bool BitmapCocoa::create(uint32_t width, uint32_t height)
{
	m_image = [[NSImage alloc] initWithSize: NSMakeSize(width, height)];
	m_imageRep = [[NSBitmapImageRep alloc]
		initWithBitmapDataPlanes: NULL
		pixelsWide: width
		pixelsHigh: height
		bitsPerSample: 8
		samplesPerPixel: 4
		hasAlpha: YES
		isPlanar: NO
		colorSpaceName: NSCalibratedRGBColorSpace
		bytesPerRow: width * 4
		bitsPerPixel: NULL];
	[m_image addRepresentation: m_imageRep];

	m_imagePreAlpha = [[NSImage alloc] initWithSize: NSMakeSize(width, height)];
	m_imageRepPreAlpha = [[NSBitmapImageRep alloc]
		initWithBitmapDataPlanes: NULL
		pixelsWide: width
		pixelsHigh: height
		bitsPerSample: 8
		samplesPerPixel: 4
		hasAlpha: YES
		isPlanar: NO
		colorSpaceName: NSCalibratedRGBColorSpace
		bytesPerRow: width * 4
		bitsPerPixel: NULL];
	[m_imagePreAlpha addRepresentation: m_imageRepPreAlpha];

	return true;
}

void BitmapCocoa::destroy()
{
	m_image = nullptr;
	m_imageRep = nullptr;
	m_imagePreAlpha = nullptr;
	m_imageRepPreAlpha = nullptr;
}

void BitmapCocoa::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
	if (!image)
		return;

	if (srcRect.left >= int(image->getWidth()) || srcRect.top >= int(image->getHeight()))
		return;
	if (srcRect.right < 0 || srcRect.bottom < 0)
		return;

	Rect rc = srcRect;

	rc.left = std::max< int >(0, rc.left);
	rc.top = std::max< int >(0, rc.top);
	rc.right = std::min< int >(image->getWidth(), rc.right);
	rc.bottom = std::min< int >(image->getHeight(), rc.bottom);

	if (rc.getWidth() <= 0 || rc.getHeight() <= 0)
		return;

	Size size = getSize();

	int width = size.cx - destPos.x;
	int height = size.cy - destPos.y;

	if (width <= 0 || height <= 0)
		return;

	if (rc.getWidth() > width)
		rc.right = rc.left + width;
	if (rc.getHeight() > height)
		rc.bottom = rc.top + height;

	bool haveAlpha = image->getPixelFormat().getAlphaBits() > 0;

	Ref< drawing::Image > sourceImage = image->clone();
	sourceImage->convert(drawing::PixelFormat::getA8B8G8R8());

	const uint32_t* sourceBits = (const uint32_t*)(sourceImage->getData());
	uint32_t* destinationBits = (uint32_t*)[m_imageRep bitmapData];
	uint32_t* destinationPreAlphaBits = (uint32_t*)[m_imageRepPreAlpha bitmapData];
	uint32_t sourceWidth = sourceImage->getWidth();

	for (int y = rc.top; y < rc.bottom; ++y)
	{
		for (int x = rc.left; x < rc.right; ++x)
		{
			uint32_t dstOffset = destPos.x + (x - rc.left) + (size.cy - (destPos.y + (y - rc.top)) - 1) * size.cx;
			uint32_t c = sourceBits[x + y * sourceWidth];

			if (!haveAlpha)
				c |= 0xff000000;

			uint32_t pa = (c & 0xff000000) >> 24;
			uint32_t pr = (c & 0x000000ff);
			uint32_t pg = (c & 0x0000ff00) >> 8;
			uint32_t pb = (c & 0x00ff0000) >> 16;

			pr = (pr * pa) >> 8;
			pg = (pg * pa) >> 8;
			pb = (pb * pa) >> 8;

			destinationBits[dstOffset] = c;
			destinationPreAlphaBits[dstOffset] = (pa << 24) | (pb << 16) | (pg << 8) | pr;
		}
	}
}

Ref< drawing::Image > BitmapCocoa::getImage() const
{
	Size size = getSize();

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getA8B8G8R8(),
		size.cx,
		size.cy
	);

	const uint32_t* sourceBits = (const uint32_t*)[m_imageRep bitmapData];
	uint32_t* destinationBits = (uint32_t*)(image->getData());

	for (int y = 0; y < size.cy; ++y)
	{
		const uint32_t* sp = &sourceBits[(size.cy - y - 1) * size.cx];
		uint32_t* dp = &destinationBits[y * size.cx];
		for (int x = 0; x < size.cx; ++x)
			*dp++ = *sp++;
	}

	return image;
}

Size BitmapCocoa::getSize() const
{
	NSSize size = [m_image size];
	return Size(size.width, size.height);
}

	}
}
