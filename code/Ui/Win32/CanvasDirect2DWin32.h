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
