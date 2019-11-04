#if defined(T_USE_GDI_PLUS)

#include <cmath>
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/CanvasGdiPlusWin32.h"
#include "Ui/Win32/Window.h"

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

	T_ASSERT(m_graphics.ptr());

	m_graphics->SetTextRenderingHint(TextRenderingHintSystemDefault);
	m_graphics->SetPixelOffsetMode(PixelOffsetModeNone);
	m_graphics->SetSmoothingMode(SmoothingModeHighSpeed);

	setForeground(Color4ub(255, 255, 255, 255));
	setBackground(Color4ub(220, 220, 220, 255));

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

void CanvasGdiPlusWin32::getAscentAndDescent(Window& hWnd, int32_t& outAscent, int32_t& outDescent) const
{
	HDC hDC = GetDC(hWnd);

	Gdiplus::Font font(hDC, hWnd.getFont());
	Gdiplus::FontFamily fontFamily;
	font.GetFamily(&fontFamily);

	UINT16 emHeight = fontFamily.GetEmHeight(FontStyleRegular);
	UINT16 ascent = fontFamily.GetCellAscent(FontStyleRegular);
	UINT16 descent = fontFamily.GetCellDescent(FontStyleRegular);

	outAscent = font.GetSize() * ascent / emHeight;
	outDescent = font.GetSize() * descent / emHeight;

	ReleaseDC(hWnd, hDC);
}

int32_t CanvasGdiPlusWin32::getAdvance(Window& hWnd, wchar_t ch, wchar_t next) const
{
	HDC hDC = GetDC(hWnd);

	Gdiplus::Graphics graphics(hDC);
	Gdiplus::Font font(hDC, hWnd.getFont());
	Gdiplus::RectF layoutRect(0.0f, 0.0f, std::numeric_limits< float >::max(), std::numeric_limits< float >::max());

	wchar_t chs[3] = { ch, next, 0 };

	Gdiplus::CharacterRange range(0, 1);
	Gdiplus::StringFormat stringFormat(StringFormat::GenericTypographic());
	stringFormat.SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoFitBlackBox);
	stringFormat.SetTrimming(StringTrimmingNone);
	stringFormat.SetMeasurableCharacterRanges(1, &range);

	Gdiplus::Region region;
	graphics.MeasureCharacterRanges(
		chs,
		-1,
		&font,
		layoutRect,
		&stringFormat,
		1,
		&region
	);

	Gdiplus::RectF rc;
	region.GetBounds(&rc, &graphics);

	ReleaseDC(hWnd, hDC);

	return rc.Width;
}

int32_t CanvasGdiPlusWin32::getLineSpacing(Window& hWnd) const
{
	HDC hDC = GetDC(hWnd);

	Gdiplus::Font font(hDC, hWnd.getFont());
	Gdiplus::FontFamily fontFamily;
	font.GetFamily(&fontFamily);

	int32_t lineSpacing = font.GetSize() * fontFamily.GetLineSpacing(FontStyleRegular) / fontFamily.GetEmHeight(FontStyleRegular);

	ReleaseDC(hWnd, hDC);
	return lineSpacing;
}

Size CanvasGdiPlusWin32::getExtent(Window& hWnd, const std::wstring& text) const
{
	Gdiplus::RectF boundingBox;

	HDC hDC = GetDC(hWnd);

	Gdiplus::Graphics graphics(hDC);
	Gdiplus::Font font(hDC, hWnd.getFont());

	Gdiplus::StringFormat stringFormat(StringFormat::GenericTypographic());
	stringFormat.SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces);
	stringFormat.SetTrimming(StringTrimmingNone);

	graphics.MeasureString(
		text.c_str(),
		(INT)text.length(),
		&font,
		Gdiplus::RectF(0, 0, 0, 0),
		&stringFormat,
		&boundingBox
	);

	ReleaseDC(hWnd, hDC);
	return Size(int(boundingBox.Width + 1), int(boundingBox.Height + 4));
}

void CanvasGdiPlusWin32::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	Gdiplus::Font font(m_hDC, m_hFont);
	Gdiplus::FontFamily fontFamily;
	font.GetFamily(&fontFamily);

	UINT16 emHeight = fontFamily.GetEmHeight(FontStyleRegular);
	UINT16 ascent = fontFamily.GetCellAscent(FontStyleRegular);
	UINT16 descent = fontFamily.GetCellDescent(FontStyleRegular);

	outAscent = font.GetSize() * ascent / emHeight;
	outDescent = font.GetSize() * descent / emHeight;
}

int32_t CanvasGdiPlusWin32::getAdvance(wchar_t ch, wchar_t next) const
{
	Gdiplus::RectF layoutRect(0.0f, 0.0f, std::numeric_limits< float >::max(), std::numeric_limits< float >::max());

	wchar_t chs[3] = { ch, next, 0 };

	Gdiplus::CharacterRange range(0, 1);
	Gdiplus::StringFormat stringFormat(StringFormat::GenericTypographic());
	stringFormat.SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoFitBlackBox);
	stringFormat.SetTrimming(StringTrimmingNone);
	stringFormat.SetMeasurableCharacterRanges(1, &range);

	Gdiplus::Region region;
	m_graphics->MeasureCharacterRanges(
		chs,
		-1,
		m_font.ptr(),
		layoutRect,
		&stringFormat,
		1,
		&region
	);

	Gdiplus::RectF rc;
	region.GetBounds(&rc, m_graphics.ptr());

	return rc.Width;
}

int32_t CanvasGdiPlusWin32::getLineSpacing() const
{
	Gdiplus::FontFamily fontFamily;
	m_font->GetFamily(&fontFamily);

	return m_font->GetSize() * fontFamily.GetLineSpacing(FontStyleRegular) / fontFamily.GetEmHeight(FontStyleRegular);
}

Size CanvasGdiPlusWin32::getExtent(const std::wstring& text) const
{
	Gdiplus::RectF boundingBox;

	if (!m_font.ptr())
		m_font.reset(new Gdiplus::Font(m_hDC, m_hFont));

	Gdiplus::StringFormat stringFormat(StringFormat::GenericTypographic());
	stringFormat.SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoFitBlackBox);
	stringFormat.SetTrimming(StringTrimmingNone);

	m_graphics->MeasureString(
		text.c_str(),
		(INT)text.length(),
		m_font.ptr(),
		Gdiplus::RectF(0, 0, 0, 0),
		&stringFormat,
		&boundingBox
	);

	return Size(int(boundingBox.Width + 1), int(boundingBox.Height + 1));
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

const IFontMetric* CanvasGdiPlusWin32::getFontMetric() const
{
	return this;
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
	if (npnts <= 32)
	{
		StaticVector< Gdiplus::Point, 32 > points(npnts);
		for (int i = 0; i < npnts; ++i)
		{
			points[i].X = pnts[i].x;
			points[i].Y = pnts[i].y;
		}
		m_graphics->DrawLines(&m_pen, &points[0], (INT)points.size());
	}
	else
	{
		std::vector< Gdiplus::Point > points(npnts);
		for (int i = 0; i < npnts; ++i)
		{
			points[i].X = pnts[i].x;
			points[i].Y = pnts[i].y;
		}
		m_graphics->DrawLines(&m_pen, &points[0], (INT)points.size());
	}
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
	if (count <= 32)
	{
		StaticVector< Gdiplus::Point, 32 > points(count);
		for (int i = 0; i < count; ++i)
		{
			points[i].X = pnts[i].x;
			points[i].Y = pnts[i].y;
		}
		m_graphics->DrawPolygon(&m_pen, &points[0], (INT)points.size());
	}
	else
	{
		std::vector< Gdiplus::Point > points(count);
		for (int i = 0; i < count; ++i)
		{
			points[i].X = pnts[i].x;
			points[i].Y = pnts[i].y;
		}
		m_graphics->DrawPolygon(&m_pen, &points[0], (INT)points.size());
	}
}

void CanvasGdiPlusWin32::fillPolygon(const Point* pnts, int count)
{
	if (count <= 32)
	{
		StaticVector< Gdiplus::Point, 32 > points(count);
		for (int i = 0; i < count; ++i)
		{
			points[i].X = pnts[i].x;
			points[i].Y = pnts[i].y;
		}
		m_graphics->FillPolygon(&m_brush, &points[0], (INT)points.size());
	}
	else
	{
		std::vector< Gdiplus::Point > points(count);
		for (int i = 0; i < count; ++i)
		{
			points[i].X = pnts[i].x;
			points[i].Y = pnts[i].y;
		}
		m_graphics->FillPolygon(&m_brush, &points[0], (INT)points.size());
	}	
}

void CanvasGdiPlusWin32::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode)
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

void CanvasGdiPlusWin32::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode)
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

	Gdiplus::StringFormat stringFormat(StringFormat::GenericTypographic());
	stringFormat.SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoFitBlackBox);
	stringFormat.SetTrimming(StringTrimmingNone);

	m_graphics->DrawString(
		text.c_str(),
		(INT)text.length(),
		m_font.ptr(),
		Gdiplus::PointF((Gdiplus::REAL)at.x, (Gdiplus::REAL)at.y),
		&stringFormat,
		&SolidBrush(m_foreGround)
	);
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
