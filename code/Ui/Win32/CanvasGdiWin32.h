/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CanvasGdiWin32_H
#define traktor_ui_CanvasGdiWin32_H

#include "Ui/Win32/CanvasWin32.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class CanvasGdiWin32 : public CanvasWin32
{
public:
	CanvasGdiWin32();

	virtual ~CanvasGdiWin32();

	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC);

	virtual void endPaint(Window& hWnd);

	virtual Size getTextExtent(Window& hWnd, const std::wstring& text) const;

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
	
	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode);

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode);

	virtual void drawText(const Point& at, const std::wstring& text);

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop);
	
	virtual Size getTextExtent(const std::wstring& text) const;

	virtual void* getSystemHandle();

private:
	PAINTSTRUCT m_ps;
	HDC m_hDC;
	bool m_ownDC;
	HBITMAP m_hOffScreenBitmap;
	uint32_t m_offScreenBitmapWidth;
	uint32_t m_offScreenBitmapHeight;
	Color4ub m_foreGround;
	Color4ub m_backGround;
	LineStyle m_lineStyle;
	int m_thickness;
	Font m_font;
	HPEN m_hOriginalPen;
	HPEN m_hPen;
	HBRUSH m_hOriginalBrush;
	HBRUSH m_hBrush;
	HFONT m_hOriginalFont;
	HFONT m_hFont;
	HRGN m_hClip;
#if defined(_DEBUG)
	int32_t m_gdiResources;
#endif

	void updatePen();

	void updateBrush();

	void updateFont();
};
	
	}
}

#endif	// traktor_ui_CanvasGdiWin32_H
