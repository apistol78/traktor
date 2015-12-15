#if defined(T_USE_GDI_PLUS)

#include <cmath>
#include "Ui/Application.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/CanvasGdiPlusWin32.h"

using namespace Gdiplus;

namespace traktor
{
	namespace ui
	{
		namespace
		{

GdiplusStartupInput s_si;
ULONG_PTR s_token = 0;

		}

CanvasGdiPlusWin32::CanvasGdiPlusWin32()
:	m_hDC(NULL)
,	m_hFont(NULL)
,	m_ownDC(false)
,	m_doubleBuffer(false)
,	m_offScreenBitmapWidth(0)
,	m_offScreenBitmapHeight(0)
,	m_foreGround(0x000000)
,	m_backGround(0xc0c0c0)
,	m_pen(Gdiplus::Color(0x000000), 1.0f)
,	m_brush(Gdiplus::Color(0xc0c0c0))
{
}

CanvasGdiPlusWin32::~CanvasGdiPlusWin32()
{
}

bool CanvasGdiPlusWin32::beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC)
{
	T_ASSERT_M (!m_hDC, L"Invalid device context handle");

	if (hDC)
	{
		m_hDC = hDC;
		m_ownDC = false;
		m_doubleBuffer = false;

		m_graphics.reset(new Graphics(m_hDC));
	}
	else
	{
		m_hDC = BeginPaint(hWnd, &m_ps);
		if (!m_hDC)
			return false;

		if (!doubleBuffer)
		{
			m_graphics.reset(new Graphics(m_hDC));
			m_doubleBuffer = false;
		}
		else
		{
			uint32_t width = m_ps.rcPaint.right - m_ps.rcPaint.left;
			uint32_t height = m_ps.rcPaint.bottom - m_ps.rcPaint.top;

			if (m_offScreenBitmap.ptr() == 0 || m_offScreenBitmapWidth < width || m_offScreenBitmapHeight < height)
			{
				m_offScreenBitmapWidth = max(m_offScreenBitmapWidth, width);
				m_offScreenBitmapHeight = max(m_offScreenBitmapHeight, height);
				m_offScreenBitmap.reset(new Bitmap(m_offScreenBitmapWidth, m_offScreenBitmapHeight));
			}

			m_graphics.reset(new Graphics(m_offScreenBitmap.ptr()));
			m_graphics->TranslateTransform(-m_ps.rcPaint.left, -m_ps.rcPaint.top);

			Gdiplus::Rect rcClip(
				m_ps.rcPaint.left,
				m_ps.rcPaint.top,
				m_ps.rcPaint.right - m_ps.rcPaint.left,
				m_ps.rcPaint.bottom - m_ps.rcPaint.top
			);
			m_graphics->SetClip(rcClip);

			m_doubleBuffer = true;
		}

		m_ownDC = true;
	}

	m_hFont = hWnd.getFont();

	T_ASSERT (m_graphics.ptr());

	m_graphics->SetTextRenderingHint(TextRenderingHintSystemDefault);
	m_graphics->SetPixelOffsetMode(PixelOffsetModeNone);
	m_graphics->SetSmoothingMode(SmoothingModeHighSpeed);

	setForeground(Color4ub(255, 255, 255, 255));
	setBackground(getSystemColor(ScButtonFace));

	return true;
}

void CanvasGdiPlusWin32::endPaint(Window& hWnd)
{
	m_font.release();
	m_graphics.release();

	if (m_doubleBuffer)
	{
		Gdiplus::Graphics graphics(m_hDC);

		uint32_t width = m_ps.rcPaint.right - m_ps.rcPaint.left;
		uint32_t height = m_ps.rcPaint.bottom - m_ps.rcPaint.top;

		graphics.DrawImage(
			m_offScreenBitmap.ptr(),
			m_ps.rcPaint.left,
			m_ps.rcPaint.top,
			0,
			0,
			width,
			height,
			Gdiplus::UnitPixel
		);
	}

	if (m_ownDC)
		EndPaint(hWnd, &m_ps);

	m_hDC = NULL;
}

Size CanvasGdiPlusWin32::getTextExtent(Window& hWnd, const std::wstring& text) const
{
	Gdiplus::RectF boundingBox;

	HDC hDC = GetDC(hWnd);

	AutoPtr< Gdiplus::Graphics > graphics(new Gdiplus::Graphics(hDC));
	AutoPtr< Gdiplus::Font > font(new Gdiplus::Font(hDC, hWnd.getFont()));

	AutoPtr< Gdiplus::StringFormat > stringFormat(StringFormat::GenericTypographic()->Clone());
	stringFormat->SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces);
	stringFormat->SetTrimming(StringTrimmingNone);

	graphics->MeasureString(
		text.c_str(),
		(INT)text.length(),
		font.ptr(),
		Gdiplus::RectF(0, 0, 0, 0),
		stringFormat.ptr(),
		&boundingBox
	);

	ReleaseDC(hWnd, hDC);

	return Size(int(boundingBox.Width), int(boundingBox.Height));
}

void CanvasGdiPlusWin32::setForeground(const Color4ub& color)
{
	m_foreGround = (uint32_t)color;
	m_pen.SetColor(m_foreGround);
}

void CanvasGdiPlusWin32::setBackground(const Color4ub& color)
{
	m_backGround = (uint32_t)color;
	m_brush.SetColor(m_backGround);
}

void CanvasGdiPlusWin32::setFont(const Font& font)
{
	m_font.reset(new Gdiplus::Font(
		font.getFace().c_str(),
		(Gdiplus::REAL)font.getPixelSize(),
		(font.isBold() ? FontStyleBold : 0) |
		(font.isItalic() ? FontStyleItalic : 0) |
		(font.isUnderline() ? FontStyleUnderline : 0),
		UnitPixel,
		NULL
	));
}

void CanvasGdiPlusWin32::setLineStyle(LineStyle lineStyle)
{
}

void CanvasGdiPlusWin32::setPenThickness(int thickness)
{
	m_pen.SetWidth((Gdiplus::REAL)thickness);
}

void CanvasGdiPlusWin32::setClipRect(const Rect& rc)
{
	m_graphics->SetClip(
		Gdiplus::Rect(rc.left, rc.top, rc.getWidth(), rc.getHeight())
	);
}

void CanvasGdiPlusWin32::resetClipRect()
{
	m_graphics->ResetClip();
}

void CanvasGdiPlusWin32::drawPixel(int x, int y, const Color4ub& c)
{
}

void CanvasGdiPlusWin32::drawLine(int x1, int y1, int x2, int y2)
{
	m_graphics->DrawLine(&m_pen, x1, y1, x2, y2);
}

void CanvasGdiPlusWin32::drawLines(const Point* pnts, int npnts)
{
	std::vector< Gdiplus::Point > points(npnts);
	for (int i = 0; i < npnts; ++i)
	{
		points[i].X = pnts[i].x;
		points[i].Y = pnts[i].y;
	}
	m_graphics->DrawLines(&m_pen, &points[0], (INT)points.size());
}

void CanvasGdiPlusWin32::fillCircle(int x, int y, float radius)
{
	m_graphics->FillEllipse(
		&m_brush,
		x - radius,
		y - radius,
		radius * 2,
		radius * 2
	);
}

void CanvasGdiPlusWin32::drawCircle(int x, int y, float radius)
{
	m_graphics->DrawEllipse(
		&m_pen,
		x - radius,
		y - radius,
		radius * 2,
		radius * 2
	);
}

void CanvasGdiPlusWin32::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
	m_graphics->DrawArc(&m_pen, x, y, w, h, start, fabs(end - start));
}

void CanvasGdiPlusWin32::drawSpline(const Point* pnts, int npnts)
{
	drawLines(pnts, npnts);
}

void CanvasGdiPlusWin32::fillRect(const Rect& rc)
{
	Rect tmp = rc.getUnified();
	m_graphics->FillRectangle(&m_brush, tmp.left, tmp.top, tmp.getWidth(), tmp.getHeight());
}

void CanvasGdiPlusWin32::fillGradientRect(const Rect& rc, bool vertical)
{
	Rect tmp = rc.getUnified();
	m_graphics->FillRectangle(
		&LinearGradientBrush(
			Gdiplus::Rect(
				tmp.left - 1, tmp.top - 1,
				tmp.getWidth() + 2, tmp.getHeight() + 2
			),
			rc.top < rc.bottom ? m_foreGround : m_backGround,
			rc.top < rc.bottom ? m_backGround : m_foreGround,
			vertical ? LinearGradientModeVertical : LinearGradientModeHorizontal
		),
		tmp.left,
		tmp.top,
		tmp.getWidth(),
		tmp.getHeight()
	);
}

void CanvasGdiPlusWin32::drawRect(const Rect& rc)
{
	Rect tmp = rc.getUnified();
	m_graphics->DrawRectangle(&m_pen, tmp.left, tmp.top, tmp.getWidth() - 1, tmp.getHeight() - 1);
}

void CanvasGdiPlusWin32::drawRoundRect(const Rect& rc, int radius)
{
	Rect tmp = rc.getUnified();
	m_graphics->DrawRectangle(&m_pen, tmp.left, tmp.top, tmp.getWidth() - 1, tmp.getHeight() - 1);
}

void CanvasGdiPlusWin32::drawPolygon(const Point* pnts, int count)
{
	std::vector< Gdiplus::Point > points(count);
	for (int i = 0; i < count; ++i)
	{
		points[i].X = pnts[i].x;
		points[i].Y = pnts[i].y;
	}
	m_graphics->DrawPolygon(&m_pen, &points[0], (INT)points.size());
}

void CanvasGdiPlusWin32::fillPolygon(const Point* pnts, int count)
{
	std::vector< Gdiplus::Point > points(count);
	for (int i = 0; i < count; ++i)
	{
		points[i].X = pnts[i].x;
		points[i].Y = pnts[i].y;
	}
	m_graphics->FillPolygon(&m_brush, &points[0], (INT)points.size());
}

void CanvasGdiPlusWin32::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	Gdiplus::Bitmap* bm = reinterpret_cast< BitmapWin32* >(bitmap)->getGdiPlusBitmap();
	if (bm)
	{
		Gdiplus::ImageAttributes attr;
		if ((blendMode & BmModulate) != 0)
		{
			Gdiplus::ColorMatrix cm =
			{
				m_backGround.GetR() / 255.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, m_backGround.GetG() / 255.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, m_backGround.GetB() / 255.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, m_backGround.GetA() / 255.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 1.0f
			};
			attr.SetColorMatrix(&cm);
		}

		m_graphics->DrawImage(
			bm,
			Gdiplus::RectF((Gdiplus::REAL)dstAt.x, (Gdiplus::REAL)dstAt.y, (Gdiplus::REAL)size.cx, (Gdiplus::REAL)size.cy),
			(Gdiplus::REAL)srcAt.x,
			(Gdiplus::REAL)srcAt.y,
			(Gdiplus::REAL)size.cx,
			(Gdiplus::REAL)size.cy,
			UnitPixel,
			&attr
		);
	}
}

void CanvasGdiPlusWin32::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	Gdiplus::Bitmap* bm = reinterpret_cast< BitmapWin32* >(bitmap)->getGdiPlusBitmap();
	if (bm)
	{
		Gdiplus::ImageAttributes attr;
		if ((blendMode & BmModulate) != 0)
		{
			Gdiplus::ColorMatrix cm =
			{
				m_backGround.GetR() / 255.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, m_backGround.GetG() / 255.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, m_backGround.GetB() / 255.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, m_backGround.GetA() / 255.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 1.0f
			};
			attr.SetColorMatrix(&cm);
		}

		m_graphics->DrawImage(
			bm,
			Gdiplus::RectF((Gdiplus::REAL)dstAt.x, (Gdiplus::REAL)dstAt.y, (Gdiplus::REAL)dstSize.cx, (Gdiplus::REAL)dstSize.cy),
			(Gdiplus::REAL)srcAt.x,
			(Gdiplus::REAL)srcAt.y,
			(Gdiplus::REAL)srcSize.cx,
			(Gdiplus::REAL)srcSize.cy,
			UnitPixel,
			&attr
		);
	}
}

void CanvasGdiPlusWin32::drawText(const Point& at, const std::wstring& text)
{
	if (!m_font.ptr())
		m_font.reset(new Gdiplus::Font(m_hDC, m_hFont));

	if (!m_stringFormat.ptr())
	{
		m_stringFormat.reset(StringFormat::GenericTypographic()->Clone());
		m_stringFormat->SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces);
		m_stringFormat->SetTrimming(StringTrimmingNone);
	}

	m_stringFormat->SetAlignment(StringAlignmentNear);
	m_stringFormat->SetLineAlignment(StringAlignmentNear);

	m_graphics->DrawString(
		text.c_str(),
		(INT)text.length(),
		m_font.ptr(),
		Gdiplus::PointF((Gdiplus::REAL)at.x, (Gdiplus::REAL)at.y),
		m_stringFormat.ptr(),
		&SolidBrush(m_foreGround)
	);
}

void CanvasGdiPlusWin32::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	if (!m_font.ptr())
		m_font.reset(new Gdiplus::Font(m_hDC, m_hFont));

	if (!m_stringFormat.ptr())
	{
		m_stringFormat.reset(StringFormat::GenericTypographic()->Clone());
		m_stringFormat->SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces);
		m_stringFormat->SetTrimming(StringTrimmingNone);
	}

	switch (halign)
	{
	case AnLeft:
		m_stringFormat->SetAlignment(StringAlignmentNear);
		break;

	case AnCenter:
		m_stringFormat->SetAlignment(StringAlignmentCenter);
		break;

	case AnRight:
		m_stringFormat->SetAlignment(StringAlignmentFar);
		break;
	}

	switch (valign)
	{
	case AnTop:
		m_stringFormat->SetLineAlignment(StringAlignmentNear);
		break;

	case AnCenter:
		m_stringFormat->SetLineAlignment(StringAlignmentCenter);
		break;

	case AnBottom:
		m_stringFormat->SetLineAlignment(StringAlignmentFar);
		break;
	}

	m_graphics->DrawString(
		text.c_str(),
		(INT)text.length(),
		m_font.ptr(),
		Gdiplus::RectF((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)rc.getWidth(), (Gdiplus::REAL)rc.getHeight()),
		m_stringFormat.ptr(),
		&SolidBrush(m_foreGround)
	);
}

Size CanvasGdiPlusWin32::getTextExtent(const std::wstring& text) const
{
	Gdiplus::RectF boundingBox;

	if (!m_font.ptr())
		m_font.reset(new Gdiplus::Font(m_hDC, m_hFont));

	if (!m_stringFormat.ptr())
	{
		m_stringFormat.reset(StringFormat::GenericTypographic()->Clone());
		m_stringFormat->SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces);
		m_stringFormat->SetTrimming(StringTrimmingNone);
	}

	m_stringFormat->SetAlignment(StringAlignmentNear);
	m_stringFormat->SetLineAlignment(StringAlignmentNear);

	m_graphics->MeasureString(
		text.c_str(),
		(INT)text.length(),
		m_font.ptr(),
		Gdiplus::RectF(0, 0, 0, 0),
		m_stringFormat.ptr(),
		&boundingBox
	);

	return Size(int(boundingBox.Width + 1), int(boundingBox.Height));
}

void* CanvasGdiPlusWin32::getSystemHandle()
{
	return m_hDC;
}

bool CanvasGdiPlusWin32::startup()
{
	GdiplusStartup(&s_token, &s_si, NULL);
	return true;
}

void CanvasGdiPlusWin32::shutdown()
{
	GdiplusShutdown(s_token);
}

	}
}

#endif	// T_USE_GDI_PLUS
