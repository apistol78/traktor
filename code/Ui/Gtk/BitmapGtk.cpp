#include "Core/Log/Log.h"
#include "Core/Math/Color4ub.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Gtk/BitmapGtk.h"

namespace traktor
{
	namespace ui
	{

bool BitmapGtk::create(uint32_t width, uint32_t height)
{
	m_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
	if (!m_surface)
	{
		T_DEBUG(L"Unable to create Pixbuf object");
		return false;
	}
	return true;
}

void BitmapGtk::destroy()
{
	m_surface.clear();
}

void BitmapGtk::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
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
	uint32_t* destinationBits = (uint32_t*)(m_surface->get_data());
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
		}
	}
}

Ref< drawing::Image > BitmapGtk::getImage() const
{
	Size size = getSize();

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		size.cx,
		size.cy
	);

	const uint32_t* sourceBits = reinterpret_cast< const uint32_t* >(m_surface->get_data());
	uint32_t* destinationBits = static_cast< uint32_t* >(image->getData());

	for (int y = 0; y < size.cy; ++y)
	{
		const uint32_t* sp = &sourceBits[(size.cy - y - 1) * size.cx];
		uint32_t* dp = &destinationBits[y * size.cx];
		for (int x = 0; x < size.cx; ++x)
			*dp++ = *sp++;
	}

	return image;
}

Size BitmapGtk::getSize() const
{
	return Size(m_surface->get_width(), m_surface->get_height());
}

void BitmapGtk::setPixel(uint32_t x, uint32_t y, const Color4ub& color)
{
	uint32_t* bits = reinterpret_cast< uint32_t* >(m_surface->get_data());
	uint32_t pitch = m_surface->get_stride() / 4;
	bits[x + y * pitch] = color.getRGBA();
}

Color4ub BitmapGtk::getPixel(uint32_t x, uint32_t y) const
{
	const uint8_t* bits = reinterpret_cast< const uint8_t* >(m_surface->get_data());
	uint32_t pitch = m_surface->get_stride();
	const uint8_t* px = &bits[x + y * pitch];
	return Color4ub(px[0], px[1], px[2], px[3]);
}

	}
}
