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
{
	m_foregroundColor = [NSColor controlTextColor];
	m_backgroundColor = [NSColor controlBackgroundColor];
	[NSBezierPath setDefaultLineWidth: 1];
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
}

void CanvasCocoa::resetClipRect()
{
}

void CanvasCocoa::drawPixel(int x, int y, const Color& c)
{
}

void CanvasCocoa::drawLine(int x1, int y1, int x2, int y2)
{
	[m_foregroundColor set];
	[NSBezierPath
		strokeLineFromPoint:makeNSPoint(Point(x1, y1))
		toPoint:makeNSPoint(Point(x2, y2))
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
	[m_backgroundColor set];
	[NSBezierPath fillRect: makeNSRect(rc)];
}

void CanvasCocoa::fillGradientRect(const Rect& rc, bool vertical)
{
	NSGradient* gradient = [[[NSGradient alloc]
		initWithColorsAndLocations:
			m_foregroundColor, (CGFloat)0.0f,
			m_backgroundColor, (CGFloat)1.0f,
			nil
		]
		autorelease];

	[gradient drawInRect: makeNSRect(rc) angle: vertical ? 90.0f : 0.0f];
}

void CanvasCocoa::drawRect(const Rect& rc)
{
	[m_foregroundColor set];
	[NSBezierPath strokeRect: makeNSRect(rc)];
}

void CanvasCocoa::drawRoundRect(const Rect& rc, int radius)
{
	[m_foregroundColor set];
	[NSBezierPath strokeRect: makeNSRect(rc)];
}

void CanvasCocoa::drawPolygon(const Point* pnts, int count)
{
}

void CanvasCocoa::fillPolygon(const Point* pnts, int count)
{
}

void CanvasCocoa::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, BlendMode blendMode)
{
	BitmapCocoa* bmc = static_cast< BitmapCocoa* >(bitmap);
	T_ASSERT (bmc);
	
	NSImage* nsi = bmc->getNSImage();
	T_ASSERT (nsi);
	
	NSRect dstRect = makeNSRect(Rect(dstAt, size));
	NSRect srcRect = makeNSRect(Rect(srcAt, size));
	
	[nsi drawInRect: dstRect fromRect: srcRect operation: NSCompositeCopy fraction: 1.0f];
}

void CanvasCocoa::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, BlendMode blendMode)
{
	BitmapCocoa* bmc = static_cast< BitmapCocoa* >(bitmap);
	T_ASSERT (bmc);
	
	NSImage* nsi = bmc->getNSImage();
	T_ASSERT (nsi);

	NSRect dstRect = makeNSRect(Rect(dstAt, dstSize));
	NSRect srcRect = makeNSRect(Rect(srcAt, srcSize));
	
	[nsi drawInRect: dstRect fromRect: srcRect operation: NSCompositeCopy fraction: 1.0f];
}

void CanvasCocoa::drawText(const Point& at, const std::wstring& text)
{
	NSString* str = makeNSString(text);
	[str drawAtPoint: makeNSPoint(at) withAttributes: NULL];
}

void CanvasCocoa::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	NSString* str = makeNSString(text);
	[str drawAtPoint: makeNSPoint(rc.getTopLeft()) withAttributes: NULL];
}

Size CanvasCocoa::getTextExtent(const std::wstring& text) const
{
	// @fixme
	return Size(text.length() * 16, 16);
}
	
	}
}
