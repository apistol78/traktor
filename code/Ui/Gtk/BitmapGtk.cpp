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
	m_pb = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, width, height);
	if (!m_pb)
	{
		T_DEBUG(L"Unable to create Pixbuf object");
		return false;
	}
	return true;
}

void BitmapGtk::destroy()
{
	m_pb.clear();
}

void BitmapGtk::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
}

Ref< drawing::Image > BitmapGtk::getImage() const
{
	Size size = getSize();

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		size.cx,
		size.cy
	);

	const uint32_t* sourceBits = (const uint32_t*)m_pb->get_pixels();
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

Size BitmapGtk::getSize() const
{
	return Size(m_pb->get_width(), m_pb->get_height());
}

void BitmapGtk::setPixel(uint32_t x, uint32_t y, const Color4ub& color)
{
}

Color4ub BitmapGtk::getPixel(uint32_t x, uint32_t y) const
{
	return Color4ub(0, 0, 0, 0);
}

	}
}
