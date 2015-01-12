#ifndef traktor_ui_CanvasWx_H
#define traktor_ui_CanvasWx_H

#include <wx/wx.h>
#include <wx/graphics.h>
#include "Ui/Itf/ICanvas.h"

namespace traktor
{
	namespace ui
	{

class CanvasWx : public ICanvas
{
public:
	bool beginPaint(wxWindow* window, bool doubleBuffer);

	void endPaint(wxWindow* window);
	
	virtual void setForeground(const Color4ub& foreground);

	virtual void setBackground(const Color4ub& background);

	virtual void setFont(const Font& font);

	virtual void setLineStyle(LineStyle lineStyle);

	virtual void setPenThickness(int thickness);

	virtual void setClipRect(const Rect& rc);

	virtual void resetClipRect();
	
	virtual void drawPixel(int x, int y, const Color4ub& c);

	virtual void drawLine(int x1, int y1, int x2, int y2);

	virtual void drawLines(const Point* pnts, int npnts);

	virtual void fillCircle(int x, int y, float radius);

	virtual void drawCircle(int x, int y, float radius);

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end);

	virtual void drawSpline(const Point* pnts, int npnts);

	virtual void fillRect(const Rect& rc);

	virtual void fillGradientRect(const Rect& rc, bool vertical = true);

	virtual void drawRect(const Rect& rc);

	virtual void drawRoundRect(const Rect& rc, int radius);

	virtual void drawPolygon(const Point* pnts, int count);

	virtual void fillPolygon(const Point* pnts, int count);
	
	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode);

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode);

	virtual void drawText(const Point& at, const std::wstring& text);

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop);
	
	virtual Size getTextExtent(const std::wstring& text) const;

	virtual void* getSystemHandle();
	
private:
	wxPaintDC* m_dc;
#if wxUSE_GRAPHICS_CONTEXT
	wxGraphicsContext* m_context;
#endif
	Color4ub m_foreGround;
	Color4ub m_backGround;
};
	
	}
}

#endif	// traktor_ui_CanvasWx_H
