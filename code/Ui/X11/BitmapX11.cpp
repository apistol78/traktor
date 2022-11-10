/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Color4ub.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/X11/BitmapX11.h"

namespace traktor
{
	namespace ui
	{

bool BitmapX11::create(uint32_t width, uint32_t height)
{
	m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	return bool(m_surface != nullptr);
}

void BitmapX11::destroy()
{
	if (m_surface != nullptr)
	{
		cairo_surface_destroy(m_surface);
		m_surface = nullptr;
	}
}

void BitmapX11::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
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
	sourceImage->convert(drawing::PixelFormat::getA8R8G8B8());

	cairo_surface_flush(m_surface);

	uint8_t* destinationBits = (uint8_t*)cairo_image_surface_get_data(m_surface);
	uint32_t destinationPitch = cairo_image_surface_get_stride(m_surface);

	Color4f c;
	for (int y = rc.top; y < rc.bottom; ++y)
	{
		for (int x = rc.left; x < rc.right; ++x)
		{
			uint32_t dstOffset = (destPos.x + (x - rc.left)) * 4 + (destPos.y + (y - rc.top)) * destinationPitch;
			uint32_t* dstBits = (uint32_t*)&destinationBits[dstOffset];

			sourceImage->getPixelUnsafe(x, y, c);

			if (haveAlpha)
				c *= c.aaa1();
			else
				c = c.rgb1();

			Color4ub u = c.toColor4ub();
			*dstBits = u.getARGB();
		}
	}

	cairo_surface_mark_dirty(m_surface);
}

Ref< drawing::Image > BitmapX11::getImage() const
{
	Size size = getSize();

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getA8R8G8B8(),
		size.cx,
		size.cy
	);

	cairo_surface_flush(m_surface);

	const uint8_t* sourceBits = reinterpret_cast< const uint8_t* >(cairo_image_surface_get_data(m_surface));
	uint32_t sourcePitch = cairo_image_surface_get_stride(m_surface);

	for (int y = 0; y < size.cy; ++y)
	{
		const uint32_t* sp = (const uint32_t*)&sourceBits[y * sourcePitch];
		for (int x = 0; x < size.cx; ++x)
		{
			float rgba[4];
			Color4ub(*sp++).getRGBA32F(rgba);
			Color4f c(rgba);
			c /= c.aaa1();
			image->setPixelUnsafe(x, y, c);
		}
	}

	return image;
}

Size BitmapX11::getSize() const
{
	int32_t w = cairo_image_surface_get_width(m_surface);
	int32_t h = cairo_image_surface_get_height(m_surface);
	return Size(w, h);
}

	}
}
