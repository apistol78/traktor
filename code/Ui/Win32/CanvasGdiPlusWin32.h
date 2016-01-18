#ifndef traktor_ui_CanvasGdiPlusWin32_H
#define traktor_ui_CanvasGdiPlusWin32_H

#if defined(T_USE_GDI_PLUS)

#include <windows.h>
#include <gdiplus.h>
#include "Core/Misc/AutoPtr.h"
#include "Ui/Win32/CanvasWin32.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class CanvasGdiPlusWin32 : public CanvasWin32
{
public:
	CanvasGdiPlusWin32();

	virtual ~CanvasGdiPlusWin32();

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
	HFONT m_hFont;
	bool m_ownDC;
	bool m_doubleBuffer;
	uint32_t m_offScreenBitmapWidth;
	uint32_t m_offScreenBitmapHeight;
	AutoPtr< Gdiplus::Bitmap > m_offScreenBitmap;
	mutable AutoPtr< Gdiplus::Graphics > m_graphics;
	mutable AutoPtr< Gdiplus::StringFormat > m_stringFormat;
	mutable AutoPtr< Gdiplus::Font > m_font;
	Gdiplus::Color m_foreGround;
	Gdiplus::Color m_backGround;
	Gdiplus::Pen m_pen;
	Gdiplus::SolidBrush m_brush;
};
	
	}
}

#endif	// T_USE_GDI_PLUS

#endif	// traktor_ui_CanvasGdiPlusWin32_H
