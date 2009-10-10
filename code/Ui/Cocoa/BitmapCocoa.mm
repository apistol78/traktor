#include "Ui/Cocoa/BitmapCocoa.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/GcNew.h"
#include "Core/Math/Color.h"

namespace traktor
{
	namespace ui
	{

BitmapCocoa::BitmapCocoa()
:	m_image(0)
,	m_imageRep(0)
{
}

bool BitmapCocoa::create(uint32_t width, uint32_t height)
{
	T_ASSERT (!m_image);
	
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
		bytesPerRow: NULL
		bitsPerPixel: NULL];
		
	[m_image addRepresentation: m_imageRep];

	return true;
}

void BitmapCocoa::destroy()
{
	[m_image release]; m_image = 0;
	[m_imageRep release]; m_imageRep = 0;
}

void BitmapCocoa::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
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

	Ref< drawing::Image > sourceImage = image->clone();
	sourceImage->convert(drawing::PixelFormat::getA8R8G8B8());

	const uint32_t* sourceBits = (const uint32_t*)(image->getData());
	uint32_t* destinationBits = (uint32_t*)[m_imageRep bitmapData];
	
	for (int y = rc.top; y < rc.bottom; ++y)
	{
		for (int x = rc.left; x < rc.right; ++x)
		{
			uint32_t dstOffset = destPos.x + (x - rc.left) + (destPos.y + (y - rc.top)) * size.cx;
			uint32_t c = sourceBits[x + y * size.cx];
			destinationBits[dstOffset] = c;
		}
	}
}

drawing::Image* BitmapCocoa::getImage() const
{
	Size size = getSize();
	
	Ref< drawing::Image > image = gc_new< drawing::Image >(
		drawing::PixelFormat::getA8R8G8B8(),
		size.cx,
		size.cy
	);
	
	const uint32_t* sourceBits = (const uint32_t*)[m_imageRep bitmapData];
	uint32_t* destinationBits = (uint32_t*)(image->getData());
	
	std::memcpy(destinationBits, sourceBits, size.cx * size.cy * sizeof(uint32_t));

	return image;
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
