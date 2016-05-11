#ifndef traktor_ui_CanvasGdiPlusWin32_H
#define traktor_ui_CanvasGdiPlusWin32_H

#if defined(T_USE_GDI_PLUS)

#include <windows.h>
// \hack As we don't want min/max defined but gdiplus.h require them
// we temporarily define them first.
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#include <gdiplus.h>
#undef min
#undef max
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

	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC) T_OVERRIDE T_FINAL;

	virtual void endPaint(Window& hWnd) T_OVERRIDE T_FINAL;

	virtual Size getTextExtent(Window& hWnd, const std::wstring& text) const T_OVERRIDE T_FINAL;
	
	virtual void setForeground(const Color4ub& foreground) T_OVERRIDE T_FINAL;

	virtual void setBackground(const Color4ub& background) T_OVERRIDE T_FINAL;

	virtual void setFont(const Font& font) T_OVERRIDE T_FINAL;

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

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop) T_OVERRIDE T_FINAL;
	
	virtual Size getTextExtent(const std::wstring& text) const T_OVERRIDE T_FINAL;

	virtual void* getSystemHandle() T_OVERRIDE T_FINAL;

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
