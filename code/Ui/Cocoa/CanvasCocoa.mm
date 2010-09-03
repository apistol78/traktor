#include "Ui/Cocoa/CanvasCocoa.h"
#include "Ui/Cocoa/BitmapCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{
		
CanvasCocoa::CanvasCocoa(NSView* view)
:	m_view(view)
,	m_foregroundColor(0)
,	m_backgroundColor(0)
,	m_clipStack(0)
{
	m_foregroundColor = [NSColor controlTextColor];
	m_backgroundColor = [NSColor controlBackgroundColor];
	[NSBezierPath setDefaultLineWidth: 1];
}

CanvasCocoa::~CanvasCocoa()
{
	while (m_clipStack > 0)
		resetClipRect();
}

void CanvasCocoa::setForeground(const Color& foreground)
{
	m_foregroundColor = makeNSColor(foreground);
}

void CanvasCocoa::setBackground(const Color& background)
{
	m_backgroundColor = makeNSColor(background);
}

void CanvasCocoa::setFont(const Font& font)
{
}

void CanvasCocoa::setLineStyle(LineStyle lineStyle)
{
}

void CanvasCocoa::setPenThickness(int thickness)
{
	[NSBezierPath setDefaultLineWidth: thickness];
}

void CanvasCocoa::setClipRect(const Rect& rc)
{
/*
	if (m_clipStack++ == 0)
	{
		NSGraphicsContext* context = [NSGraphicsContext currentContext];
		[context saveGraphicsState];
	}
	
	NSRectClip(makeNSRect(rc));
*/
}

void CanvasCocoa::resetClipRect()
{
/*
	if (--m_clipStack == 0)
	{
		NSGraphicsContext* context = [NSGraphicsContext currentContext];
		[context restoreGraphicsState];
	}
*/
}

void CanvasCocoa::drawPixel(int x, int y, const Color& c)
{
}

void CanvasCocoa::drawLine(int x1, int y1, int x2, int y2)
{
	NSPoint p1 = makeNSPoint(Point(x1, y1));
	NSPoint p2 = makeNSPoint(Point(x2, y2));
	
	p1.x += 0.5; p1.y += 0.5;
	p2.x += 0.5; p2.y += 0.5;

	[m_foregroundColor set];
	[NSBezierPath
		strokeLineFromPoint: p1
		toPoint: p2
	];
}

void CanvasCocoa::drawLines(const Point* pnts, int npnts)
{
	for (int i = 0; i < npnts - 1; ++i)
		drawLine(pnts[i].x, pnts[i].y, pnts[i + 1].x, pnts[i + 1].y);
}

void CanvasCocoa::fillCircle(int x, int y, float radius)
{
}

void CanvasCocoa::drawCircle(int x, int y, float radius)
{
}

void CanvasCocoa::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
}

void CanvasCocoa::drawSpline(const Point* pnts, int npnts)
{
}

void CanvasCocoa::fillRect(const Rect& rc)
{
	NSRect nrc = makeNSRect(rc);
	nrc = NSOffsetRect(nrc, 0.5, 0.5);

	[m_backgroundColor set];
	[NSBezierPath fillRect: nrc];
}

void CanvasCocoa::fillGradientRect(const Rect& rc, bool vertical)
{
	NSRect nrc = makeNSRect(rc);
	nrc = NSOffsetRect(nrc, 0.5, 0.5);

	NSGradient* gradient = [[[NSGradient alloc]
		initWithColorsAndLocations:
			m_foregroundColor, (CGFloat)0.0f,
			m_backgroundColor, (CGFloat)1.0f,
			nil
		] autorelease];

	[gradient drawInRect: nrc angle: vertical ? 90.0f : 0.0f];
}

void CanvasCocoa::drawRect(const Rect& rc)
{
	NSRect nrc = makeNSRect(rc);
	nrc = NSOffsetRect(nrc, 0.5, 0.5);

	[m_foregroundColor set];
	[NSBezierPath strokeRect: nrc];
}

void CanvasCocoa::drawRoundRect(const Rect& rc, int radius)
{
	NSRect nrc = makeNSRect(rc);
	nrc = NSOffsetRect(nrc, 0.5, 0.5);

	[m_foregroundColor set];
	[NSBezierPath strokeRect: nrc];
}

void CanvasCocoa::drawPolygon(const Point* pnts, int count)
{
}

void CanvasCocoa::fillPolygon(const Point* pnts, int count)
{
}

void CanvasCocoa::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	BitmapCocoa* bmc = static_cast< BitmapCocoa* >(bitmap);
	T_ASSERT (bmc);
	
	NSRect dstRect = makeNSRect(Rect(dstAt, size));
	NSRect srcRect = makeNSRect(Rect(srcAt, size));
	
	if (blendMode == 0)
		[bmc->getNSImage() drawInRect: dstRect fromRect: srcRect operation: NSCompositeCopy fraction: 1.0f];
	else if ((blendMode & BmAlpha) != 0)
		[bmc->getNSImagePreAlpha() drawInRect: dstRect fromRect: srcRect operation: NSCompositeSourceOver fraction: 1.0f];
}

void CanvasCocoa::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	BitmapCocoa* bmc = static_cast< BitmapCocoa* >(bitmap);
	T_ASSERT (bmc);
	
	NSRect dstRect = makeNSRect(Rect(dstAt, dstSize));
	NSRect srcRect = makeNSRect(Rect(srcAt, srcSize));
	
	if (blendMode == 0)
		[bmc->getNSImage() drawInRect: dstRect fromRect: srcRect operation: NSCompositeCopy fraction: 1.0f];
	else if ((blendMode & BmAlpha) != 0)
		[bmc->getNSImagePreAlpha() drawInRect: dstRect fromRect: srcRect operation: NSCompositeSourceOver fraction: 1.0f];
}

void CanvasCocoa::drawText(const Point& at, const std::wstring& text)
{
	NSFont* font = [NSFont controlContentFontOfSize: 11];
	
	NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
	[attributes setObject: font forKey:NSFontAttributeName];
	[attributes setObject: m_foregroundColor forKey: NSForegroundColorAttributeName];

	NSString* str = makeNSString(text);
	[str drawAtPoint: makeNSPoint(at) withAttributes: attributes];
}

void CanvasCocoa::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	NSFont* font = [NSFont controlContentFontOfSize: 11];
	
	NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
	[attributes setObject: font forKey: NSFontAttributeName];
	[attributes setObject: m_foregroundColor forKey: NSForegroundColorAttributeName];
	
	NSString* str = makeNSString(text);
	
	NSRect nrc = makeNSRect(rc);
	NSSize nsz = [str sizeWithAttributes: NULL];
	
	if (halign == AnCenter)
		nrc.origin.x += (nrc.size.width - nsz.width) / 2;
	else if (halign == AnRight)
		nrc.origin.x = (nrc.origin.x + nrc.size.width) - nsz.width;
	
	if (valign == AnCenter)
		nrc.origin.y += (nrc.size.height - nsz.height) / 2;
	else if (valign == AnBottom)
		nrc.origin.y = (nrc.origin.y + nrc.size.height) - nsz.height;

	[str drawInRect: nrc withAttributes: attributes];
}

Size CanvasCocoa::getTextExtent(const std::wstring& text) const
{
	NSFont* font = [NSFont controlContentFontOfSize: 11];
	
	NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
	[attributes setObject: font forKey:NSFontAttributeName];

	NSString* str = makeNSString(text);
	NSSize size = [str sizeWithAttributes: attributes];
	
	return fromNSSize(size);
}
	
	}
}
