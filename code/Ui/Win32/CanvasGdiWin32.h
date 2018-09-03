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

	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC) T_OVERRIDE T_FINAL;

	virtual void endPaint(Window& hWnd) T_OVERRIDE T_FINAL;

	virtual void getAscentAndDescent(Window& hWnd, int32_t& outAscent, int32_t& outDescent) const T_OVERRIDE T_FINAL;

	virtual int32_t getAdvance(Window& hWnd, wchar_t ch, wchar_t next) const T_OVERRIDE T_FINAL;

	virtual int32_t getLineSpacing(Window& hWnd) const T_OVERRIDE T_FINAL;

	virtual Size getExtent(Window& hWnd, const std::wstring& text) const T_OVERRIDE T_FINAL;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const T_OVERRIDE T_FINAL;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const T_OVERRIDE T_FINAL;

	virtual int32_t getLineSpacing() const T_OVERRIDE T_FINAL;

	virtual Size getExtent(const std::wstring& text) const T_OVERRIDE T_FINAL;

	// ICanvas

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

	virtual void fillGradientRect(const Rect& rc, bool vertical = true) T_OVERRIDE T_FINAL;

	virtual void drawRect(const Rect& rc) T_OVERRIDE T_FINAL;

	virtual void drawRoundRect(const Rect& rc, int radius) T_OVERRIDE T_FINAL;

	virtual void drawPolygon(const Point* pnts, int count) T_OVERRIDE T_FINAL;

	virtual void fillPolygon(const Point* pnts, int count) T_OVERRIDE T_FINAL;
	
	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode) T_OVERRIDE T_FINAL;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode) T_OVERRIDE T_FINAL;

	virtual void drawText(const Point& at, const std::wstring& text) T_OVERRIDE T_FINAL;

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
