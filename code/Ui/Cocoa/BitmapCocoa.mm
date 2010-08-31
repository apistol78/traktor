#include "Core/Math/Color.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Cocoa/BitmapCocoa.h"
#include "Ui/Cocoa/NSDebugAutoreleasePool.h"

namespace traktor
{
	namespace ui
	{

BitmapCocoa::BitmapCocoa()
:	m_image(0)
,	m_imageRep(0)
,	m_imagePreAlpha(0)
,	m_imageRepPreAlpha(0)
{
}

bool BitmapCocoa::create(uint32_t width, uint32_t height)
{
	NSDebugAutoreleasePool* pool = [[NSDebugAutoreleasePool alloc] init];

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

	[pool release];
	return true;
}

void BitmapCocoa::destroy()
{
	if (m_image)
		[m_image release];
	
	if (m_imageRep)
		[m_imageRep release];
		
	if (m_imagePreAlpha)
		[m_imagePreAlpha release];
	
	if (m_imageRepPreAlpha)
		[m_imageRepPreAlpha release];
		
	m_image = 0;
	m_imageRep = 0;
	m_imagePreAlpha = 0;
	m_imageRepPreAlpha = 0;
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

void BitmapCocoa::setPixel(uint32_t x, uint32_t y, const Color& color)
{
}

Color BitmapCocoa::getPixel(uint32_t x, uint32_t y) const
{
	Size size = getSize();
	
	if (x >= size.cx || y >= size.cx)
		return Color(0, 0, 0);

	const uint32_t* sourceBits = (const uint32_t*)[m_imageRep bitmapData];
	uint32_t c = sourceBits[x + (size.cy - y - 1) * size.cx];

	uint32_t pr = (c & 0x000000ff);
	uint32_t pg = (c & 0x0000ff00) >> 8;
	uint32_t pb = (c & 0x00ff0000) >> 16;

	return Color(pr, pg, pb);
}

	}
}
