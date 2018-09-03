/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CanvasDirect2DWin32_H
#define traktor_ui_CanvasDirect2DWin32_H

#if defined(T_USE_DIRECT2D)

#include <map>
#include <d2d1.h>
#include <dwrite.h>
#include "Core/Misc/ComRef.h"
#include "Ui/Win32/CanvasWin32.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class CanvasDirect2DWin32 : public CanvasWin32
{
public:
	CanvasDirect2DWin32();

	virtual ~CanvasDirect2DWin32();

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

	static bool startup();

	static void shutdown();

private:
	PAINTSTRUCT m_ps;
	HDC m_hDC;
	bool m_ownDC;
	ComRef< ID2D1HwndRenderTarget > m_d2dRenderTarget;
	ComRef< ID2D1SolidColorBrush > m_d2dForegroundBrush;
	ComRef< ID2D1SolidColorBrush > m_d2dBackgroundBrush;
	D2D1_GRADIENT_STOP m_gradientStops[2];
	ComRef< ID2D1GradientStopCollection > m_d2dGradientStops;
	ComRef< IDWriteTextFormat > m_dwTextFormat;
	ComRef< IDWriteFont > m_dwFont;
	DWRITE_FONT_METRICS m_fontMetrics;
	std::map< int32_t, ComRef< ID2D1Bitmap > > m_d2dBitmaps;
	float m_strokeWidth;
	bool m_underline;
	bool m_clip;

	ID2D1Bitmap* getCachedBitmap(const ISystemBitmap* bm);

	void flushCachedBitmaps();
};
	
	}
}

#endif	// T_USE_DIRECT2D

#endif	// traktor_ui_CanvasDirect2DWin32_H
