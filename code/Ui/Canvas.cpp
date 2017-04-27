/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Canvas", Canvas, Object)

Canvas::Canvas(ICanvas* canvas)
:	m_canvas(canvas)
{
	T_ASSERT_M (m_canvas, L"Invalid canvas implementation");
}

void Canvas::setForeground(const Color4ub& foreground)
{
	m_foreground = foreground;
	m_canvas->setForeground(m_foreground);
}

const Color4ub& Canvas::getForeground() const
{
	return m_foreground;
}

void Canvas::setBackground(const Color4ub& background)
{
	m_background = background;
	m_canvas->setBackground(m_background);
}

const Color4ub& Canvas::getBackground() const
{
	return m_background;
}

void Canvas::setFont(const Font& font)
{
	m_canvas->setFont(font);
}

void Canvas::setLineStyle(LineStyle lineStyle)
{
	m_canvas->setLineStyle(lineStyle);
}

void Canvas::setPenThickness(int thickness)
{
	m_canvas->setPenThickness(thickness);
}

void Canvas::setClipRect(const Rect& rc)
{
	m_canvas->setClipRect(rc);
}

void Canvas::resetClipRect()
{
	m_canvas->resetClipRect();
}

void Canvas::drawPixel(int x, int y)
{
	m_canvas->drawPixel(x, y, m_foreground);
}

void Canvas::drawPixel(const Point& p)
{
	m_canvas->drawPixel(p.x, p.y, m_foreground);
}

void Canvas::drawPixel(int x, int y, const Color4ub& c)
{
	m_canvas->drawPixel(x, y, c);
}

void Canvas::drawPixel(const Point& p, const Color4ub& c)
{
	m_canvas->drawPixel(p.x, p.y, c);
}

void Canvas::drawLine(int x1, int y1, int x2, int y2)
{
	m_canvas->drawLine(x1, y1, x2, y2);
}

void Canvas::drawLine(const Point& start, const Point& end)
{
	m_canvas->drawLine(start.x, start.y, end.x, end.y);
}

void Canvas::drawLines(const std::vector< Point >& pnts)
{
	m_canvas->drawLines(&pnts[0], int(pnts.size()));
}

void Canvas::drawLines(const Point* pnts, int npnts)
{
	m_canvas->drawLines(pnts, npnts);
}

void Canvas::fillCircle(const Point& p, float radius)
{
	m_canvas->fillCircle(p.x, p.y, radius);
}

void Canvas::drawCircle(const Point& p, float radius)
{
	m_canvas->drawCircle(p.x, p.y, radius);
}

void Canvas::drawEllipticArc(const Point& p, const Size& s, float start, float end)
{
	m_canvas->drawEllipticArc(p.x, p.y, s.cx, s.cy, start, end);
}

void Canvas::drawSpline(const std::vector< Point >& pnts)
{
	m_canvas->drawSpline(&pnts[0], int(pnts.size()));
}

void Canvas::drawSpline(const Point* pnts, int npnts)
{
	m_canvas->drawSpline(pnts, npnts);
}

void Canvas::fillRect(const Rect& rc)
{
	m_canvas->fillRect(rc);
}

void Canvas::fillGradientRect(const Rect& rc, bool vertical)
{
	m_canvas->fillGradientRect(rc, vertical);
}

void Canvas::drawRect(const Rect& rc)
{
	m_canvas->drawRect(rc);
}

void Canvas::drawRoundRect(const Rect& rc, int radius)
{
	m_canvas->drawRoundRect(rc, radius);
}

void Canvas::drawPolygon(const Point* pnts, int count)
{
	m_canvas->drawPolygon(pnts, count);
}

void Canvas::fillPolygon(const Point* pnts, int count)
{
	m_canvas->fillPolygon(pnts, count);
}

void Canvas::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	if (bitmap && bitmap->getSystemBitmap())
		m_canvas->drawBitmap(dstAt, srcAt, size, bitmap->getSystemBitmap(), blendMode);
}

void Canvas::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	if (bitmap && bitmap->getSystemBitmap())
		m_canvas->drawBitmap(dstAt, dstSize, srcAt, srcSize, bitmap->getSystemBitmap(), blendMode);
}

void Canvas::drawText(const Point& at, const std::wstring& text)
{
	m_canvas->drawText(at, text);
}

void Canvas::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	m_canvas->drawText(rc, text, halign, valign);
}

Size Canvas::getTextExtent(const std::wstring& text) const
{
	return m_canvas->getTextExtent(text);
}

	}
}
