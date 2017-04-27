/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Envelope.h"
#include "Core/Math/Vector2.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/CanvasGdiWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

COLORREF getColorRef(const Color4ub& c)
{
	return RGB(
		c.r,
		c.g,
		c.b
	);
}

		}

CanvasGdiWin32::CanvasGdiWin32()
:	m_hDC(NULL)
,	m_hOffScreenBitmap(NULL)
,	m_offScreenBitmapWidth(0)
,	m_offScreenBitmapHeight(0)
,	m_lineStyle(LsSolid)
,	m_thickness(0)
,	m_hOriginalPen(NULL)
,	m_hPen(NULL)
,	m_hOriginalBrush(NULL)
,	m_hBrush(NULL)
,	m_hOriginalFont(NULL)
,	m_hFont(NULL)
,	m_hClip(NULL)
{
}

CanvasGdiWin32::~CanvasGdiWin32()
{
	if (m_hOffScreenBitmap != NULL)
	{
		DeleteObject(m_hOffScreenBitmap);
		m_hOffScreenBitmap = NULL;
	}
}

bool CanvasGdiWin32::beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC)
{
	T_ASSERT_M (!m_hDC, L"Invalid device context handle");

#if defined(_DEBUG)
	m_gdiResources = (int32_t)GetGuiResources(GetCurrentProcess(), 0);
	if (m_hOffScreenBitmap == NULL)
		m_gdiResources++;
#endif

	if (hDC)
	{
		m_hDC = hDC;
		m_ownDC = false;
	}
	else
	{
		if (!doubleBuffer)
		{
			m_hDC = BeginPaint(hWnd, &m_ps);
			if (!m_hDC)
				return false;
		}
		else
		{
			HDC hPaintDC = BeginPaint(hWnd, &m_ps);
			if (!hPaintDC)
				return false;

			m_hDC = CreateCompatibleDC(hPaintDC);
			if (!m_hDC)
			{
				EndPaint(hWnd, &m_ps);
				return false;
			}

			m_hClip = CreateRectRgnIndirect(&m_ps.rcPaint);
			SelectClipRgn(m_hDC, m_hClip);

			RECT rcInner;
			GetClientRect(hWnd, &rcInner);

			uint32_t width = uint32_t(rcInner.right - rcInner.left);
			uint32_t height = uint32_t(rcInner.bottom - rcInner.top);

			if (m_hOffScreenBitmap == NULL || m_offScreenBitmapWidth != width || m_offScreenBitmapHeight != height)
			{
				if (m_hOffScreenBitmap != NULL)
				{
					DeleteObject(m_hOffScreenBitmap);
					m_hOffScreenBitmap = NULL;
				}

				m_hOffScreenBitmap = CreateCompatibleBitmap(hPaintDC, width, height);
				if (!m_hOffScreenBitmap)
				{
					EndPaint(hWnd, &m_ps);
					DeleteDC(m_hDC);
					m_hDC = NULL;
					return false;
				}

				m_offScreenBitmapWidth = width;
				m_offScreenBitmapHeight = height;
			}

			SelectObject(m_hDC, m_hOffScreenBitmap);
		}
		m_ownDC = true;
	}

	m_foreGround = Color4ub(0, 0, 0);
	m_backGround = Color4ub(0, 0, 0);
	m_lineStyle = LsSolid;
	m_thickness = 0;

	m_hPen = CreatePen(PS_SOLID, 0, RGB(0, 0, 0));
	m_hOriginalPen = (HPEN)SelectObject(m_hDC, m_hPen);

	m_hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	m_hOriginalBrush = (HBRUSH)SelectObject(m_hDC, m_hBrush);

	m_hFont = NULL;
	m_hOriginalFont = (HFONT)SelectObject(m_hDC, hWnd.getFont());

	m_font = Font();

	return true;
}

void CanvasGdiWin32::endPaint(Window& hWnd)
{
	T_ASSERT_M (m_hDC != NULL, L"Invalid device context handle");

	SelectObject(m_hDC, m_hOriginalPen);
	if (m_hPen != NULL)
	{
		DeleteObject(m_hPen);
		m_hPen = NULL;
	}

	SelectObject(m_hDC, m_hOriginalBrush);
	if (m_hBrush != NULL)
	{
		DeleteObject(m_hBrush);
		m_hBrush = NULL;
	}

	SelectObject(m_hDC, m_hOriginalFont);
	if (m_hFont != NULL)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	resetClipRect();

	if (m_hOffScreenBitmap)
	{
		BitBlt(
			m_ps.hdc,
			m_ps.rcPaint.left,
			m_ps.rcPaint.top,
			m_ps.rcPaint.right - m_ps.rcPaint.left,
			m_ps.rcPaint.bottom - m_ps.rcPaint.top,
			m_hDC,
			m_ps.rcPaint.left,
			m_ps.rcPaint.top,
			SRCCOPY
		);

		DeleteDC(m_hDC);
	}

	if (m_ownDC)
		EndPaint(hWnd, &m_ps);

	m_hDC = NULL;

#if defined(_DEBUG)
	int32_t gdiResources = (int32_t)GetGuiResources(GetCurrentProcess(), 0);
	int32_t gdiLeak = gdiResources - m_gdiResources;
	if (gdiLeak > 0)
		T_DEBUG(L"GDI " << gdiLeak << L" objects leaked (pre: " << m_gdiResources << L", post: " << gdiResources << L")");
#endif
}

Size CanvasGdiWin32::getTextExtent(Window& hWnd, const std::wstring& text) const
{
	SIZE size = { 0, 0 };
	HDC hDC = GetDC(hWnd);
	HGDIOBJ hOldFont = SelectObject(hDC, hWnd.getFont());
	GetTextExtentPoint32(hDC, wstots(text).c_str(), int(text.length()), &size);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hWnd, hDC);
	return Size(size.cx, size.cy);
}

void CanvasGdiWin32::setForeground(const Color4ub& color)
{
	if (color != m_foreGround)
	{
		m_foreGround = color;
		updatePen();

		SetTextColor(m_hDC, getColorRef(m_foreGround));
	}
}

void CanvasGdiWin32::setBackground(const Color4ub& color)
{
	if (color != m_backGround)
	{
		m_backGround = color;
		updateBrush();
	}
}

void CanvasGdiWin32::setFont(const Font& font)
{
	if (font != m_font)
	{
		m_font = font;
		updateFont();
	}
}

void CanvasGdiWin32::setLineStyle(LineStyle lineStyle)
{
	if (lineStyle != m_lineStyle)
	{
		m_lineStyle = lineStyle;
		updatePen();
	}
}

void CanvasGdiWin32::setPenThickness(int thickness)
{
	if (thickness != m_thickness)
	{
		m_thickness = thickness;
		updatePen();
	}
}

void CanvasGdiWin32::setClipRect(const Rect& rc)
{
	resetClipRect();
	m_hClip = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
	SelectClipRgn(m_hDC, m_hClip);
}

void CanvasGdiWin32::resetClipRect()
{
	SelectClipRgn(m_hDC, NULL);
	if (m_hClip)
	{
		DeleteObject(m_hClip);
		m_hClip = NULL;
	}
}

void CanvasGdiWin32::drawPixel(int x, int y, const Color4ub& c)
{
	SetPixelV(m_hDC, x, y, getColorRef(c));
}

void CanvasGdiWin32::drawLine(int x1, int y1, int x2, int y2)
{
	MoveToEx(m_hDC, x1, y1, NULL);
	LineTo(m_hDC, x2, y2);
	SetPixelV(m_hDC, x2, y2, getColorRef(m_foreGround));
}

void CanvasGdiWin32::drawLines(const Point* pnts, int npnts)
{
	if (!pnts || npnts <= 1)
		return;

	MoveToEx(m_hDC, pnts[0].x, pnts[0].y, NULL);
	for (int i = 1; i < npnts; ++i)
		LineTo(m_hDC, pnts[i].x, pnts[i].y);
}

void CanvasGdiWin32::fillCircle(int x, int y, float radius)
{
	HGDIOBJ hCurrentPen = SelectObject(m_hDC, (HPEN)GetStockObject(NULL_PEN));
	Ellipse(m_hDC, x - int(radius), y - int(radius), x + int(radius), y + int(radius));
	SelectObject(m_hDC, hCurrentPen);
}

void CanvasGdiWin32::drawCircle(int x, int y, float radius)
{
	HGDIOBJ hCurrentBrush = SelectObject(m_hDC, (HBRUSH)GetStockObject(NULL_BRUSH));
	Ellipse(m_hDC, x - int(radius), y - int(radius), x + int(radius), y + int(radius));
	SelectObject(m_hDC, hCurrentBrush);
}

void CanvasGdiWin32::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
}

void CanvasGdiWin32::drawSpline(const Point* pnts, int npnts)
{
	if (!pnts || npnts <= 1)
		return;

	MoveToEx(m_hDC, pnts[0].x, pnts[0].y, NULL);
	for (int i = 1; i < npnts; ++i)
		LineTo(m_hDC, int(pnts[i].x), int(pnts[i].y));
}

void CanvasGdiWin32::fillRect(const Rect& rc)
{
	RECT wrc = { rc.left, rc.top, rc.right, rc.bottom };
	FillRect(m_hDC, &wrc, m_hBrush);
}

void CanvasGdiWin32::fillGradientRect(const Rect& rc, bool vertical)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT rect;

	vert[0].x      = rc.left;
	vert[0].y      = rc.top;
	vert[0].Red    = m_foreGround.r << 8;
	vert[0].Green  = m_foreGround.g << 8;
	vert[0].Blue   = m_foreGround.b << 8;
	vert[0].Alpha  = 0x0000;

	vert[1].x      = rc.right;
	vert[1].y      = rc.bottom;
	vert[1].Red    = m_backGround.r << 8;
	vert[1].Green  = m_backGround.g << 8;
	vert[1].Blue   = m_backGround.b << 8;
	vert[1].Alpha  = 0x0000;

	rect.UpperLeft  = 0;
	rect.LowerRight = 1;
	GradientFill(m_hDC, vert, 2, &rect, 1, vertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

void CanvasGdiWin32::drawRect(const Rect& rc)
{
	MoveToEx(m_hDC, rc.left, rc.top, NULL);
	LineTo(m_hDC, rc.right - 1, rc.top);
	LineTo(m_hDC, rc.right - 1, rc.bottom - 1);
	LineTo(m_hDC, rc.left, rc.bottom - 1);
	LineTo(m_hDC, rc.left, rc.top);
}

void CanvasGdiWin32::drawRoundRect(const Rect& rc, int radius)
{
	HGDIOBJ hCurrentBrush = SelectObject(m_hDC, (HBRUSH)GetStockObject(NULL_BRUSH));
	RoundRect(m_hDC, rc.left, rc.top, rc.right, rc.bottom, radius * 2, radius * 2);
	SelectObject(m_hDC, hCurrentBrush);
}

void CanvasGdiWin32::drawPolygon(const Point* pnts, int count)
{
	if (!pnts || count <= 2)
		return;

	std::vector< POINT > wpnts(count);
	for (int i = 0; i < count; ++i)
	{
		wpnts[i].x = pnts[i].x;
		wpnts[i].y = pnts[i].y;
	}

	HGDIOBJ hCurrentBrush = SelectObject(m_hDC, (HBRUSH)GetStockObject(NULL_BRUSH));
	Polygon(m_hDC, &wpnts[0], count);
	SelectObject(m_hDC, hCurrentBrush);
}

void CanvasGdiWin32::fillPolygon(const Point* pnts, int count)
{
	if (!pnts || count <= 2)
		return;

	std::vector< POINT > wpnts(count);
	for (int i = 0; i < count; ++i)
	{
		wpnts[i].x = pnts[i].x;
		wpnts[i].y = pnts[i].y;
	}

	HGDIOBJ hCurrentPen = SelectObject(m_hDC, (HPEN)GetStockObject(NULL_PEN));
	Polygon(m_hDC, &wpnts[0], count);
	SelectObject(m_hDC, hCurrentPen);
}

void CanvasGdiWin32::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode)
{
	if (!bitmap)
		return;

	HDC hImageDC = CreateCompatibleDC(m_hDC);
	
	if ((blendMode & BmAlpha) == 0)
	{
		HBITMAP hBitmap = reinterpret_cast< BitmapWin32* >(bitmap)->getHBitmap();
		if (!hBitmap)
		{
			DeleteDC(hImageDC);
			return;
		}

		HGDIOBJ hCurrBitmap = SelectObject(hImageDC, hBitmap);

		BitBlt(
			m_hDC,
			dstAt.x,
			dstAt.y,
			size.cx,
			size.cy, hImageDC, srcAt.x, srcAt.y, SRCCOPY
		);

		SelectObject(hImageDC, hCurrBitmap);
	}
	else
	{
		HBITMAP hBitmap = reinterpret_cast< BitmapWin32* >(bitmap)->getHBitmapPreMulAlpha();
		if (!hBitmap)
		{
			DeleteDC(hImageDC);
			return;
		}

		HGDIOBJ hCurrBitmap = SelectObject(hImageDC, hBitmap);

		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 0xff;
		bf.AlphaFormat = AC_SRC_ALPHA;

		AlphaBlend(
			m_hDC,
			dstAt.x,
			dstAt.y,
			size.cx,
			size.cy,
			hImageDC,
			srcAt.x,
			srcAt.y,
			size.cx,
			size.cy,
			bf
		);

		SelectObject(hImageDC, hCurrBitmap);
	}

	DeleteDC(hImageDC);
}

void CanvasGdiWin32::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode)
{
	if (!bitmap)
		return;

	HDC hImageDC = CreateCompatibleDC(m_hDC);

	if ((blendMode & BmAlpha) == 0)
	{
		HBITMAP hBitmap = reinterpret_cast< BitmapWin32* >(bitmap)->getHBitmap();
		if (!hBitmap)
		{
			DeleteDC(hImageDC);
			return;
		}

		HGDIOBJ hCurrBitmap = SelectObject(hImageDC, hBitmap);

		StretchBlt(
			m_hDC,
			dstAt.x,
			dstAt.y,
			dstSize.cx,
			dstSize.cy,
			hImageDC,
			srcAt.x,
			srcAt.y,
			srcSize.cx,
			srcSize.cy,
			SRCCOPY
		);

		SelectObject(hImageDC, hCurrBitmap);
	}
	else
	{
		HBITMAP hBitmap = reinterpret_cast< BitmapWin32* >(bitmap)->getHBitmapPreMulAlpha();
		if (!hBitmap)
		{
			DeleteDC(hImageDC);
			return;
		}

		HGDIOBJ hCurrBitmap = SelectObject(hImageDC, hBitmap);

		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 0xff;
		bf.AlphaFormat = AC_SRC_ALPHA;

		AlphaBlend(
			m_hDC,
			dstAt.x,
			dstAt.y,
			dstSize.cx,
			dstSize.cy,
			hImageDC,
			srcAt.x,
			srcAt.y,
			srcSize.cx,
			srcSize.cy,
			bf
		);

		SelectObject(hImageDC, hCurrBitmap);
	}

	DeleteDC(hImageDC);
}

void CanvasGdiWin32::drawText(const Point& at, const std::wstring& text)
{
	RECT wrc = { at.x, at.y, 0, 0 };
	UINT format = DT_SINGLELINE | DT_NOCLIP;

	SetBkMode(m_hDC, TRANSPARENT);
	DrawText(m_hDC, wstots(text).c_str(), int(text.length()), &wrc, format);
}

void CanvasGdiWin32::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	RECT wrc = { rc.left, rc.top, rc.right, rc.bottom };
	UINT format = DT_SINGLELINE;
	
	switch (halign)
	{
	case AnLeft:
		format |= DT_LEFT;
		break;

	case AnCenter:
		format |= DT_CENTER;
		break;

	case AnRight:
		format |= DT_RIGHT;
		break;
	}

	switch (valign)
	{
	case AnTop:
		format |= DT_TOP;
		break;

	case AnCenter:
		format |= DT_VCENTER;
		break;

	case AnBottom:
		format |= DT_BOTTOM;
		break;
	}

	SetBkMode(m_hDC, TRANSPARENT);
	DrawText(m_hDC, wstots(text).c_str(), int(text.length()), &wrc, format);
}

Size CanvasGdiWin32::getTextExtent(const std::wstring& text) const
{
	SIZE size;
	GetTextExtentPoint32(m_hDC, wstots(text).c_str(), int(text.length()), &size);
	return Size(size.cx, size.cy);
}

void* CanvasGdiWin32::getSystemHandle()
{
	return m_hDC;
}

void CanvasGdiWin32::updatePen()
{
	int style = PS_SOLID;

	if (m_lineStyle == LsDot)
		style = PS_DOT;
	else if (m_lineStyle == LsDotDash)
		style = PS_DASHDOT;

	if (m_hPen != NULL)
		DeleteObject(m_hPen);

	m_hPen = CreatePen(style, m_thickness, getColorRef(m_foreGround));
	SelectObject(m_hDC, m_hPen);
}

void CanvasGdiWin32::updateBrush()
{
	if (m_hBrush != NULL)
		DeleteObject(m_hBrush);

	m_hBrush = CreateSolidBrush(getColorRef(m_backGround));
	SelectObject(m_hDC, m_hBrush);
}

void CanvasGdiWin32::updateFont()
{
	LOGFONT lf;

	int32_t dip = m_font.getSize();
	float inches = dip / 96.0f;
	float logical = inches * getSystemDPI();

	std::memset(&lf, 0, sizeof(lf));
	lf.lfHeight = -int32_t(logical + 0.5f);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = m_font.isBold() ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = m_font.isItalic() ? TRUE : FALSE;
	lf.lfUnderline = m_font.isUnderline() ? TRUE : FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, LF_FACESIZE, wstots(m_font.getFace()).c_str());

	if (m_hFont != NULL)
		DeleteObject(m_hFont);

	m_hFont = CreateFontIndirect(&lf);
	SelectObject(m_hDC, m_hFont);
}

	}
}
