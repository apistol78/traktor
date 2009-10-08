#include "Ui/Cocoa/BitmapCocoa.h"
#include "Core/Math/Color.h"

namespace traktor
{
	namespace ui
	{

BitmapCocoa::BitmapCocoa()
:	m_image(0)
{
}

bool BitmapCocoa::create(uint32_t width, uint32_t height)
{
	T_ASSERT (!m_image);
	
	m_image = [[NSImage alloc] initWithSize: NSMakeSize(width, height)];

	return true;
}

void BitmapCocoa::destroy()
{
	[m_image release];
	m_image = 0;
}

void BitmapCocoa::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
}

drawing::Image* BitmapCocoa::getImage() const
{
	return 0;
}

Size BitmapCocoa::getSize() const
{
	NSSize size = [m_image size];
	return Size(size.width, size.height);
}

void BitmapCocoa::setPixel(uint32_t x, uint32_t y, const Color& color)
{
}

Color BitmapCocoa::getPixel(uint32_t x, uint32_t y) const
{
	return Color(0, 0, 0);
}

	}
}
