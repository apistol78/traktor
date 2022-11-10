/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Cocoa/CanvasCocoa.h"
#include "Ui/Cocoa/BitmapCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

CanvasCocoa::CanvasCocoa(NSFont* font)
:	m_font(font)
,	m_haveClipper(false)
{
	m_foregroundColor = [NSColor controlTextColor];
	m_backgroundColor = [NSColor windowBackgroundColor];
	[NSBezierPath setDefaultLineWidth: 1];

	// Prevent antialias from start.
	NSGraphicsContext* context = [NSGraphicsContext currentContext];
	context.shouldAntialias = NO;
}

CanvasCocoa::~CanvasCocoa()
{
	resetClipRect();
}

void CanvasCocoa::setForeground(const Color4ub& foreground)
{
	m_foregroundColor = makeNSColor(foreground);
}

void CanvasCocoa::setBackground(const Color4ub& background)
{
	m_backgroundColor = makeNSColor(background);
}

void CanvasCocoa::setFont(const Font& font)
{
	m_font = [NSFont
		fontWithName: makeNSString(font.getFace())
		size: (CGFloat)font.getSize()
	];
}

const IFontMetric* CanvasCocoa::getFontMetric() const
{
	return this;
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
	resetClipRect();

	NSGraphicsContext* context = [NSGraphicsContext currentContext];
	[context saveGraphicsState];
	m_haveClipper = true;

	NSRectClip(makeNSRect(rc));
}

void CanvasCocoa::resetClipRect()
{
	if (m_haveClipper)
	{
		NSGraphicsContext* context = [NSGraphicsContext currentContext];
		[context restoreGraphicsState];
		m_haveClipper = false;
	}
}

void CanvasCocoa::drawPixel(int x, int y, const Color4ub& c)
{
}

void CanvasCocoa::drawLine(int x1, int y1, int x2, int y2)
{
	NSPoint p1 = makeNSPoint(Point(x1, y1));
	NSPoint p2 = makeNSPoint(Point(x2, y2));

	p1.x += 0.5f; p1.y += 0.5f;
	p2.x += 0.5f; p2.y += 0.5f;

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
	[m_backgroundColor set];

	NSBezierPath* path = [[NSBezierPath alloc] init];
	[path appendBezierPathWithArcWithCenter:
		makeNSPoint(Point(x, y))
		radius: radius
		startAngle: 0.0f
		endAngle: 360.0f
	];
	[path fill];
}

void CanvasCocoa::drawCircle(int x, int y, float radius)
{
	[m_foregroundColor set];

	NSBezierPath* path = [[NSBezierPath alloc] init];
	[path appendBezierPathWithArcWithCenter:
		makeNSPoint(Point(x, y))
		radius: radius
		startAngle: 0.0f
		endAngle: 360.0f
	];
	[path stroke];
}

void CanvasCocoa::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
}

void CanvasCocoa::drawSpline(const Point* pnts, int npnts)
{
}

void CanvasCocoa::fillRect(const Rect& rc)
{
	NSRect nrc = makeNSRect(rc.getUnified());

	nrc.origin.x += 0.5f;
	nrc.origin.y += 0.5f;
	nrc.size.width -= 1.0f;
	nrc.size.height -= 1.0f;

	[m_backgroundColor set];
	[NSBezierPath fillRect: nrc];
}

void CanvasCocoa::fillGradientRect(const Rect& rc, bool vertical)
{
	NSRect nrc = makeNSRect(rc.getUnified());

	nrc.origin.x += 0.5f;
	nrc.origin.y += 0.5f;
	nrc.size.width -= 1.0f;
	nrc.size.height -= 1.0f;

	NSGradient* gradient = [[NSGradient alloc]
		initWithColorsAndLocations:
			m_foregroundColor, (CGFloat)0.0f,
			m_backgroundColor, (CGFloat)1.0f,
			nil
		];

	[gradient drawInRect: nrc angle: vertical ? 90.0f : 0.0f];
}

void CanvasCocoa::drawRect(const Rect& rc)
{
	NSRect nrc = makeNSRect(rc.getUnified());

	nrc.origin.x += 0.5f;
	nrc.origin.y += 0.5f;
	nrc.size.width -= 1.0f;
	nrc.size.height -= 1.0f;

	[m_foregroundColor set];
	[NSBezierPath strokeRect: nrc];
}

void CanvasCocoa::drawRoundRect(const Rect& rc, int radius)
{
	NSRect nrc = makeNSRect(rc.getUnified());

	nrc.origin.x += 0.5f;
	nrc.origin.y += 0.5f;
	nrc.size.width -= 1.0f;
	nrc.size.height -= 1.0f;

	[m_foregroundColor set];
	[NSBezierPath strokeRect: nrc];
}

void CanvasCocoa::drawPolygon(const Point* pnts, int count)
{
	if (count < 2)
		return;

	for (int i = 0; i < count - 1; ++i)
		drawLine(pnts[i].x, pnts[i].y, pnts[i + 1].x, pnts[i + 1].y);

	drawLine(pnts[count - 1].x, pnts[count - 1].y, pnts[0].x, pnts[0].y);
}

void CanvasCocoa::fillPolygon(const Point* pnts, int count)
{
	if (count <= 2)
		return;

	[m_backgroundColor set];

	NSBezierPath* path = [[NSBezierPath alloc] init];
	[path moveToPoint: makeNSPoint(pnts[0])];
	for (int i = 1; i < count; ++i)
		[path lineToPoint: makeNSPoint(pnts[i])];
	[path fill];
}

void CanvasCocoa::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode)
{
	BitmapCocoa* bmc = static_cast< BitmapCocoa* >(bitmap);
	T_ASSERT (bmc);

	NSRect dstRect = makeNSRect(Rect(dstAt, size));
	NSRect srcRect = makeNSRect(Rect(srcAt, size));

	if (blendMode == 0)
		[bmc->getNSImage() drawInRect: dstRect fromRect: srcRect operation: NSCompositingOperationCopy fraction: 1.0f];
	else if ((blendMode & BmAlpha) != 0)
		[bmc->getNSImagePreAlpha() drawInRect: dstRect fromRect: srcRect operation: NSCompositingOperationSourceOver fraction: 1.0f];
}

void CanvasCocoa::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode)
{
	BitmapCocoa* bmc = static_cast< BitmapCocoa* >(bitmap);
	T_ASSERT (bmc);

	NSRect dstRect = makeNSRect(Rect(dstAt, dstSize));
	NSRect srcRect = makeNSRect(Rect(srcAt, srcSize));

	if (blendMode == 0)
		[bmc->getNSImage() drawInRect: dstRect fromRect: srcRect operation: NSCompositingOperationCopy fraction: 1.0f];
	else if ((blendMode & BmAlpha) != 0)
		[bmc->getNSImagePreAlpha() drawInRect: dstRect fromRect: srcRect operation: NSCompositingOperationSourceOver fraction: 1.0f];
}

void CanvasCocoa::drawText(const Point& at, const std::wstring& text)
{
	NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
	[attributes setObject: m_font forKey:NSFontAttributeName];
	[attributes setObject: m_foregroundColor forKey: NSForegroundColorAttributeName];

	NSGraphicsContext* context = [NSGraphicsContext currentContext];
	context.shouldAntialias = YES;

	NSPoint nat = makeNSPoint(at);
	NSString* str = makeNSString(text);
	[str drawAtPoint: nat withAttributes: attributes];

	context.shouldAntialias = NO;
}

void* CanvasCocoa::getSystemHandle()
{
	T_FATAL_ERROR;
	return nullptr;
}

void CanvasCocoa::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	float a =  [m_font ascender];
	float b = -[m_font descender];
	float c =  [m_font leading];

	outAscent = (int32_t)a;
	outDescent = (int32_t)(b + c);
}

int32_t CanvasCocoa::getAdvance(wchar_t ch, wchar_t next) const
{
	wchar_t s[] = { ch, 0 };
	return getExtent(s).cx;
}

int32_t CanvasCocoa::getLineSpacing() const
{
	return 0;
}

Size CanvasCocoa::getExtent(const std::wstring& text) const
{
	NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
	[attributes setObject: m_font forKey:NSFontAttributeName];

	NSString* str = makeNSString(text);
	NSSize size = [str sizeWithAttributes: attributes];

	return fromNSSize(size);
}

	}
}
