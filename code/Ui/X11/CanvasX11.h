#pragma once

#include <cairo.h>
#include "Ui/Itf/ICanvas.h"
#include "Ui/Itf/IFontMetric.h"

namespace traktor
{
	namespace ui
	{

class CanvasX11
:	public ICanvas
,	public IFontMetric
{
public:
	CanvasX11(cairo_t* cr);

	virtual ~CanvasX11();

	virtual void setForeground(const Color4ub& foreground) T_OVERRIDE T_FINAL;

	virtual void setBackground(const Color4ub& background) T_OVERRIDE T_FINAL;

	virtual void setFont(const Font& font) T_OVERRIDE T_FINAL;

	virtual const IFontMetric* getFontMetric() const T_OVERRIDE T_FINAL;

	virtual void setLineStyle(LineStyle lineStyle) T_OVERRIDE T_FINAL;

	virtual void setPenThickness(int thickness) T_OVERRIDE T_FINAL;

	virtual void setClipRect(const Rect& rc) T_OVERRIDE T_FINAL;

	virtual void resetClipRect() T_OVERRIDE T_FINAL;

	virtual void drawPixel(int x, int y, const Color4ub& c) T_OVERRIDE T_FINAL;

	virtual void drawLine(int x1, int y1, int x2, int y2) T_OVERRIDE T_FINAL;

	virtual void drawLines(const Point* pnts, int npnts) T_OVERRIDE T_FINAL;

	virtual void fillCircle(int x, int y, float radius) T_OVERRIDE T_FINAL;

	virtual void drawCircle(int x, int y, float radius) T_OVERRIDE T_FINAL;

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end) T_OVERRIDE T_FINAL;

	virtual void drawSpline(const Point* pnts, int npnts) T_OVERRIDE T_FINAL;

	virtual void fillRect(const Rect& rc) T_OVERRIDE T_FINAL;

	virtual void fillGradientRect(const Rect& rc, bool vertical) T_OVERRIDE T_FINAL;

	virtual void drawRect(const Rect& rc) T_OVERRIDE T_FINAL;

	virtual void drawRoundRect(const Rect& rc, int radius) T_OVERRIDE T_FINAL;

	virtual void drawPolygon(const Point* pnts, int count) T_OVERRIDE T_FINAL;

	virtual void fillPolygon(const Point* pnts, int count) T_OVERRIDE T_FINAL;

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode) T_OVERRIDE T_FINAL;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode) T_OVERRIDE T_FINAL;

	virtual void drawText(const Point& at, const std::wstring& text) T_OVERRIDE T_FINAL;

	virtual void* getSystemHandle() T_OVERRIDE T_FINAL;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const T_OVERRIDE T_FINAL;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const T_OVERRIDE T_FINAL;

	virtual int32_t getLineSpacing() const T_OVERRIDE T_FINAL;

	virtual Size getExtent(const std::wstring& text) const T_OVERRIDE T_FINAL;

private:
	cairo_t* m_cr;
	Color4ub m_foreground;
	Color4ub m_background;
	int32_t m_thickness;
	bool m_underline;
};

	}
}
