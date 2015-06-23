#if defined(T_USE_DIRECT2D)

#include <limits>
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/CanvasDirect2DWin32.h"

#undef max

namespace traktor
{
	namespace ui
	{
		namespace
		{

ComRef< ID2D1Factory > s_d2dFactory;
ComRef< IDWriteFactory > s_dwFactory;

		}

CanvasDirect2DWin32::CanvasDirect2DWin32()
:	m_hDC(0)
,	m_ownDC(false)
,	m_strokeWidth(1.0f)
,	m_underline(false)
,	m_clip(false)
{
	m_gradientStops[0].color = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);
	m_gradientStops[0].position = 0.0f;
	m_gradientStops[1].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	m_gradientStops[1].position = 1.0f;
}

CanvasDirect2DWin32::~CanvasDirect2DWin32()
{
}

bool CanvasDirect2DWin32::beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC)
{
	HRESULT hr;

	if (hDC)
	{
		m_hDC = hDC;
		m_ownDC = false;
	}
	else
	{
		m_hDC = BeginPaint(hWnd, &m_ps);
		if (!m_hDC)
			return false;

		m_ownDC = true;
	}

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	int32_t width = rcClient.right - rcClient.left;
	int32_t height = rcClient.bottom - rcClient.top;

	if (width <= 0 || height <= 0)
	{
		m_d2dRenderTarget.release();
		return false;
	}

	bool renderTargetValid = false;

	if (m_d2dRenderTarget)
	{
		D2D1_SIZE_U size = m_d2dRenderTarget->GetPixelSize();
		renderTargetValid = (size.width == width && size.height == height);
	}

	if (!renderTargetValid)
	{
		flushCachedBitmaps();

		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		hr = s_d2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&m_d2dRenderTarget.getAssign()
		);
		if (FAILED(hr))
			return false;
	}

	T_ASSERT (m_d2dRenderTarget);

	// Force DPI to be 96 as DPI handling is performed outside of Canvas.
	m_d2dRenderTarget->SetDpi(96, 96);

	m_d2dRenderTarget->BeginDraw();
	m_d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Yellow));

	setForeground(Color4ub(0, 0, 0, 255));
	setBackground(Color4ub(255, 255, 255, 255));

	m_strokeWidth = 1.0f;

	LOGFONT lf;
	if (!GetObject(hWnd.getFont(), sizeof(lf), &lf))
		return false;

	setFont(Font(
		lf.lfFaceName,
		lf.lfHeight,
		bool(lf.lfWeight == FW_BOLD),
		bool(lf.lfItalic == TRUE),
		bool(lf.lfUnderline == TRUE)
	));

	return true;
}

void CanvasDirect2DWin32::endPaint(Window& hWnd)
{
	HRESULT hr;

	m_d2dForegroundBrush.release();
	m_d2dBackgroundBrush.release();
	m_d2dGradientStops.release();
	m_dwTextFormat.release();

	hr = m_d2dRenderTarget->EndDraw();
	if (FAILED(hr))
	{
		flushCachedBitmaps();
		m_d2dRenderTarget.release();
	}

	if (m_ownDC)
		EndPaint(hWnd, &m_ps);

	m_hDC = NULL;
}

Size CanvasDirect2DWin32::getTextExtent(Window& hWnd, const std::wstring& text) const
{
	LOGFONT lf;
	if (!GetObject(hWnd.getFont(), sizeof(lf), &lf))
		return Size(0, 0);

	ComRef< IDWriteTextFormat > dwTextFormat;
	s_dwFactory->CreateTextFormat(
		lf.lfFaceName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		std::abs(lf.lfHeight),
		L"",
		&dwTextFormat.getAssign()
	);
	if (!dwTextFormat)
		return Size(0, 0);

	dwTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	ComRef< IDWriteTextLayout > dwLayout;
	s_dwFactory->CreateTextLayout(
		text.c_str(),
		text.length(),
		dwTextFormat,
		std::numeric_limits< FLOAT >::max(),
		std::numeric_limits< FLOAT >::max(),
		&dwLayout.getAssign()
	);
	if (!dwLayout)
		return Size(0, 0);

	DWRITE_TEXT_METRICS dwtm;
	dwLayout->GetMetrics(&dwtm);

	return Size(
		dwtm.width,
		dwtm.height
	);
}

void CanvasDirect2DWin32::setForeground(const Color4ub& foreground)
{
	m_d2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(foreground, foreground.a / 255.0f),
		&m_d2dForegroundBrush.getAssign()
	);

	m_gradientStops[0].color = D2D1::ColorF(foreground, foreground.a / 255.0f);
	m_d2dGradientStops.release();
}

void CanvasDirect2DWin32::setBackground(const Color4ub& background)
{
	m_d2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(background, background.a / 255.0f),
		&m_d2dBackgroundBrush.getAssign()
	);

	m_gradientStops[1].color = D2D1::ColorF(background, background.a / 255.0f);
	m_d2dGradientStops.release();
}

void CanvasDirect2DWin32::setFont(const Font& font)
{
	int32_t size = font.getSize();
	if (size < 0)
		size = -size;

	s_dwFactory->CreateTextFormat(
		font.getFace().c_str(),
		NULL,
		font.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
		font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		size,
		L"",
		&m_dwTextFormat.getAssign()
	);

	if (m_dwTextFormat)
		m_dwTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	m_underline = font.isUnderline();
}

void CanvasDirect2DWin32::setLineStyle(LineStyle lineStyle)
{
}

void CanvasDirect2DWin32::setPenThickness(int thickness)
{
	m_strokeWidth = thickness;
}

void CanvasDirect2DWin32::setClipRect(const Rect& rc)
{
	resetClipRect();

	Rect rc2 = rc.getUnified();
	if (rc2.getWidth() <= 0 || rc2.getHeight() <= 0)
		return;

	m_d2dRenderTarget->PushAxisAlignedClip(
		D2D1::RectF(rc2.left, rc2.top, rc2.right, rc2.bottom),
		D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
	);
	m_clip = true;
}

void CanvasDirect2DWin32::resetClipRect()
{
	if (m_clip)
	{
		m_d2dRenderTarget->PopAxisAlignedClip();
		m_clip = false;
	}
}

void CanvasDirect2DWin32::drawPixel(int x, int y, const Color4ub& c)
{
}

void CanvasDirect2DWin32::drawLine(int x1, int y1, int x2, int y2)
{
	m_d2dRenderTarget->DrawLine(
		D2D1::Point2F(x1, y1),
		D2D1::Point2F(x2, y2),
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::drawLines(const Point* pnts, int npnts)
{
	if (npnts < 2)
		return;

	HRESULT hr;

	ComRef< ID2D1PathGeometry > d2dPathGeometry;
	hr = s_d2dFactory->CreatePathGeometry(&d2dPathGeometry.getAssign());
	if (FAILED(hr))
		return;

	ComRef< ID2D1GeometrySink > d2dGeometrySink;
	hr = d2dPathGeometry->Open(&d2dGeometrySink.getAssign());
	if (FAILED(hr))
		return;

	d2dGeometrySink->BeginFigure(
		D2D1::Point2F(pnts[0].x, pnts[0].y),
		D2D1_FIGURE_BEGIN_HOLLOW
	);

	for (int i = 1; i < npnts; ++i)
	{
		d2dGeometrySink->AddLine(
			D2D1::Point2F(pnts[i].x, pnts[i].y)
		);
	}

	d2dGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
	d2dGeometrySink->Close();

	m_d2dRenderTarget->DrawGeometry(
		d2dPathGeometry,
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::fillCircle(int x, int y, float radius)
{
	m_d2dRenderTarget->FillEllipse(
		D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
		m_d2dBackgroundBrush
	);
}

void CanvasDirect2DWin32::drawCircle(int x, int y, float radius)
{
	m_d2dRenderTarget->DrawEllipse(
		D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
}

void CanvasDirect2DWin32::drawSpline(const Point* pnts, int npnts)
{
}

void CanvasDirect2DWin32::fillRect(const Rect& rc)
{
	Rect rc2 = rc.getUnified();
	if (rc2.getWidth() <= 0 || rc2.getHeight() <= 0)
		return;

	m_d2dRenderTarget->FillRectangle(
		D2D1::RectF(rc2.left, rc2.top, rc2.right, rc2.bottom),
		m_d2dBackgroundBrush
	);
}

void CanvasDirect2DWin32::fillGradientRect(const Rect& rc, bool vertical)
{
	HRESULT hr;

	Rect rc2 = rc.getUnified();
	if (rc2.getWidth() <= 0 || rc2.getHeight() <= 0)
		return;

	if (!m_d2dGradientStops)
	{
		hr = m_d2dRenderTarget->CreateGradientStopCollection(m_gradientStops, sizeof_array(m_gradientStops), &m_d2dGradientStops.getAssign());
		if (FAILED(hr))
			return;
	}

	ComRef< ID2D1LinearGradientBrush > d2dGradientBrush;
	if (vertical)
	{
		hr = m_d2dRenderTarget->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(rc2.left, rc2.top),
				D2D1::Point2F(rc2.left, rc2.bottom)
			),
			m_d2dGradientStops,
			&d2dGradientBrush.getAssign()
		);
	}
	else
	{
		hr = m_d2dRenderTarget->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(rc2.left, rc2.top),
				D2D1::Point2F(rc2.right, rc2.top)
			),
			m_d2dGradientStops,
			&d2dGradientBrush.getAssign()
		);
	}
	if (FAILED(hr))
		return;

	m_d2dRenderTarget->FillRectangle(
		D2D1::RectF(rc2.left, rc2.top, rc2.right, rc2.bottom),
		d2dGradientBrush
	);
}

void CanvasDirect2DWin32::drawRect(const Rect& rc)
{
	Rect rc2 = rc.getUnified();
	if (rc2.getWidth() <= 0 || rc2.getHeight() <= 0)
		return;

	m_d2dRenderTarget->DrawRectangle(
		D2D1::RectF(rc2.left, rc2.top, rc2.right, rc2.bottom),
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::drawRoundRect(const Rect& rc, int radius)
{
	Rect rc2 = rc.getUnified();
	if (rc2.getWidth() <= 0 || rc2.getHeight() <= 0)
		return;

	m_d2dRenderTarget->DrawRoundedRectangle(
		D2D1::RoundedRect(
			D2D1::RectF(rc2.left, rc2.top, rc2.right, rc2.bottom),
			radius,
			radius
		),
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::drawPolygon(const Point* pnts, int npnts)
{
	if (npnts < 2)
		return;

	HRESULT hr;

	ComRef< ID2D1PathGeometry > d2dPathGeometry;
	hr = s_d2dFactory->CreatePathGeometry(&d2dPathGeometry.getAssign());
	if (FAILED(hr))
		return;

	ComRef< ID2D1GeometrySink > d2dGeometrySink;
	hr = d2dPathGeometry->Open(&d2dGeometrySink.getAssign());
	if (FAILED(hr))
		return;

	d2dGeometrySink->BeginFigure(
		D2D1::Point2F(pnts[0].x, pnts[0].y),
		D2D1_FIGURE_BEGIN_HOLLOW
	);

	for (int i = 1; i < npnts; ++i)
	{
		d2dGeometrySink->AddLine(
			D2D1::Point2F(pnts[i].x, pnts[i].y)
		);
	}

	d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
	d2dGeometrySink->Close();

	m_d2dRenderTarget->DrawGeometry(
		d2dPathGeometry,
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::fillPolygon(const Point* pnts, int npnts)
{
	if (npnts < 2)
		return;

	HRESULT hr;

	ComRef< ID2D1PathGeometry > d2dPathGeometry;
	hr = s_d2dFactory->CreatePathGeometry(&d2dPathGeometry.getAssign());
	if (FAILED(hr))
		return;

	ComRef< ID2D1GeometrySink > d2dGeometrySink;
	hr = d2dPathGeometry->Open(&d2dGeometrySink.getAssign());
	if (FAILED(hr))
		return;

	d2dGeometrySink->BeginFigure(
		D2D1::Point2F(pnts[0].x, pnts[0].y),
		D2D1_FIGURE_BEGIN_FILLED
	);

	for (int i = 1; i < npnts; ++i)
	{
		d2dGeometrySink->AddLine(
			D2D1::Point2F(pnts[i].x, pnts[i].y)
		);
	}

	d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
	d2dGeometrySink->Close();

	m_d2dRenderTarget->FillGeometry(
		d2dPathGeometry,
		m_d2dBackgroundBrush
	);
}

void CanvasDirect2DWin32::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	drawBitmap(dstAt, size, srcAt, size, bitmap, blendMode);
}

void CanvasDirect2DWin32::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	ID2D1Bitmap* bm = getCachedBitmap(bitmap);
	if (!bm)
		return;

	m_d2dRenderTarget->DrawBitmap(
		bm,
		D2D1::RectF(
			dstAt.x, dstAt.y,
			dstAt.x + dstSize.cx, dstAt.y + dstSize.cy
		),
		1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
		D2D1::RectF(
			srcAt.x, srcAt.y,
			srcAt.x + srcSize.cx, srcAt.y + srcSize.cy
		)
	);
}

void CanvasDirect2DWin32::drawText(const Point& at, const std::wstring& text)
{
	if (!m_dwTextFormat)
		return;

	m_dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	m_dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	ComRef< IDWriteTextLayout > dwLayout;
	s_dwFactory->CreateTextLayout(
		text.c_str(),
		text.length(),
		m_dwTextFormat,
		std::numeric_limits< FLOAT >::max(),
		std::numeric_limits< FLOAT >::max(),
		&dwLayout.getAssign()
	);
	if (!dwLayout)
		return;

	if (m_underline)
	{
		DWRITE_TEXT_RANGE range;
		range.startPosition = 0;
		range.length = text.length();
		dwLayout->SetUnderline(TRUE, range);
	}

	m_d2dRenderTarget->DrawTextLayout(
		D2D1::Point2F(at.x, at.y),
		dwLayout,
		m_d2dForegroundBrush
	);
}

void CanvasDirect2DWin32::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	Rect rc2 = rc.getUnified();
	if (rc2.getWidth() <= 0 || rc2.getHeight() <= 0)
		return;

	if (!m_dwTextFormat)
		return;

	switch (halign)
	{
	case AnLeft:
		m_dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		break;

	case AnCenter:
		m_dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		break;

	case AnRight:
		m_dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		break;
	}

	switch (valign)
	{
	case AnTop:
		m_dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		break;

	case AnCenter:
		m_dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		break;

	case AnBottom:
		m_dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		break;
	}

	ComRef< IDWriteTextLayout > dwLayout;
	s_dwFactory->CreateTextLayout(
		text.c_str(),
		text.length(),
		m_dwTextFormat,
		rc2.getWidth(),
		rc2.getHeight(),
		&dwLayout.getAssign()
	);
	if (!dwLayout)
		return;

	if (m_underline)
	{
		DWRITE_TEXT_RANGE range;
		range.startPosition = 0;
		range.length = text.length();
		dwLayout->SetUnderline(TRUE, range);
	}

	m_d2dRenderTarget->DrawTextLayout(
		D2D1::Point2F(rc2.left, rc2.top),
		dwLayout,
		m_d2dForegroundBrush
	);
}

Size CanvasDirect2DWin32::getTextExtent(const std::wstring& text) const
{
	ComRef< IDWriteTextLayout > dwLayout;
	s_dwFactory->CreateTextLayout(
		text.c_str(),
		text.length(),
		m_dwTextFormat,
		std::numeric_limits< FLOAT >::max(),
		std::numeric_limits< FLOAT >::max(),
		&dwLayout.getAssign()
	);
	if (!dwLayout)
		return Size(0, 0);

	DWRITE_TEXT_METRICS dwtm;
	dwLayout->GetMetrics(&dwtm);

	return Size(
		dwtm.width,
		dwtm.height
	);
}

void* CanvasDirect2DWin32::getSystemHandle()
{
	return m_hDC;
}

bool CanvasDirect2DWin32::startup()
{
	HRESULT hr;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &s_d2dFactory.getAssign());
	if (FAILED(hr))
		return false;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&s_dwFactory.getAssign());
	if (FAILED(hr))
		return false;

	return true;
}

void CanvasDirect2DWin32::shutdown()
{
	s_dwFactory.release();
	s_d2dFactory.release();
}

ID2D1Bitmap* CanvasDirect2DWin32::getCachedBitmap(const IBitmap* bm)
{
	std::map< const IBitmap*, ComRef< ID2D1Bitmap > >::const_iterator i = m_d2dBitmaps.find(bm);
	if (i != m_d2dBitmaps.end())
		return i->second;

	const BitmapWin32* bmw32 = reinterpret_cast< const BitmapWin32* >(bm);
	Size size = bmw32->getSize();

	const uint32_t* colorBits = reinterpret_cast< const uint32_t* >(bmw32->getBitsPerMulAlpha());
	AutoArrayPtr< uint32_t > bits(new uint32_t [size.cx * size.cy]);

	for (uint32_t y = 0; y < size.cy; ++y)
	{
		uint32_t srcOffset = (size.cy - y - 1) * size.cx;
		uint32_t dstOffset = y * size.cx;

		for (uint32_t x = 0; x < size.cx; ++x)
			bits[dstOffset + x] = colorBits[srcOffset + x];
	}

	D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
	D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(pixelFormat);

	ComRef< ID2D1Bitmap > d2dBitmap;
	HRESULT hr;

	hr = m_d2dRenderTarget->CreateBitmap(
		D2D1::SizeU(size.cx, size.cy),
		bits.c_ptr(),
		size.cx * 4,
		bitmapProps,
		&d2dBitmap.getAssign()
	);
	if (FAILED(hr))
		return 0;

	m_d2dBitmaps[bm] = d2dBitmap;
	return d2dBitmap;
}

void CanvasDirect2DWin32::flushCachedBitmaps()
{
	m_d2dBitmaps.clear();
}

	}
}

#endif
