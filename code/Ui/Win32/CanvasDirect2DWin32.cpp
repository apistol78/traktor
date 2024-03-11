/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(T_USE_DIRECT2D)

#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Atomic.h"
#include "Ui/Application.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/CanvasDirect2DWin32.h"
#include "Ui/Win32/UtilitiesWin32.h"
#include "Ui/Win32/Window.h"

#undef max

namespace traktor::ui
{
	namespace
	{

ComRef< ID2D1Factory > s_d2dFactory;
ComRef< IDWriteFactory > s_dwFactory;
int32_t s_instanceCount = 0;

	}

CanvasDirect2DWin32::CanvasDirect2DWin32()
{
	Atomic::increment(s_instanceCount);

	m_gradientStops[0].color = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);
	m_gradientStops[0].position = 0.0f;
	m_gradientStops[1].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	m_gradientStops[1].position = 1.0f;
}

CanvasDirect2DWin32::~CanvasDirect2DWin32()
{
	Atomic::decrement(s_instanceCount);
}

bool CanvasDirect2DWin32::beginPaint(Window& hWnd, const Font& font, bool doubleBuffer, HDC hDC)
{
	HRESULT hr;

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	const int32_t width = rcClient.right - rcClient.left;
	const int32_t height = rcClient.bottom - rcClient.top;

	if (width <= 0 || height <= 0)
	{
		m_d2dTintEffect.release();
		m_d2dDeviceContext.release();
		m_d2dRenderTarget.release();
		return false;
	}

	if (!m_d2dRenderTarget)
	{
		flushCachedBitmaps();

		const D2D1_SIZE_U size = D2D1::SizeU(width, height);
		hr = s_d2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
			&m_d2dRenderTarget.getAssign()
		);
		if (FAILED(hr))
			return false;

		m_d2dRenderTarget->QueryInterface(&m_d2dDeviceContext.getAssign());
		m_d2dDeviceContext->CreateEffect(CLSID_D2D1Tint, &m_d2dTintEffect.getAssign());
	}
	else
	{
		const D2D1_SIZE_U size = D2D1::SizeU(width, height);
		hr = m_d2dRenderTarget->Resize(size);
		if (FAILED(hr))
			return false;
	}

	// Force DPI to be 96 as DPI handling is performed outside of Canvas.
	m_d2dRenderTarget->SetDpi(96, 96);

	m_d2dRenderTarget->BeginDraw();
	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	setForeground(Color4ub(0, 0, 0, 255));
	setBackground(Color4ub(255, 255, 255, 255));
	setFont(font);

	m_strokeWidth = 1.0f;
	m_inPaint = true;

	// Ensure font is recreated when dpi change.
	if (hWnd.dpi() != m_dpi)
	{
		m_dwFont.release();
		m_dwTextFormat.release();
		m_dpi = hWnd.dpi();
	}

	return true;
}

void CanvasDirect2DWin32::endPaint(Window& hWnd)
{
	HRESULT hr;

	resetClipRect();

	m_d2dForegroundBrush.release();
	m_d2dBackgroundBrush.release();
	m_d2dGradientStops.release();

	hr = m_d2dRenderTarget->EndDraw();
	if (FAILED(hr))
	{
		flushCachedBitmaps();
		m_d2dRenderTarget.release();
	}

	m_inPaint = false;
}

void CanvasDirect2DWin32::getAscentAndDescent(Window& hWnd, const Font& font, int32_t& outAscent, int32_t& outDescent) const
{
	if (!m_inPaint)
	{
		outAscent = 0;
		outDescent = 0;

		const int32_t dpi = hWnd.dpi();
		const int32_t fontSize = (font.getSize().get() * dpi) / 96.0f;

		ComRef< IDWriteTextFormat > dwTextFormat;
		s_dwFactory->CreateTextFormat(
			font.getFace().c_str(),
			NULL,
			font.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
			font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			L"",
			&dwTextFormat.getAssign()
		);
		if (!dwTextFormat)
			return;

		dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		dwTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

		ComRef< IDWriteFontCollection > collection;
		dwTextFormat->GetFontCollection(&collection.getAssign());

		UINT32 findex;
		BOOL exists;
		collection->FindFamilyName(font.getFace().c_str(), &findex, &exists);
		T_FATAL_ASSERT(exists);

		ComRef< IDWriteFontFamily > ffamily;
		collection->GetFontFamily(findex, &ffamily.getAssign());
		if (!ffamily)
			return;

		ComRef< IDWriteFont > dwFont;
		ffamily->GetFirstMatchingFont(
			dwTextFormat->GetFontWeight(),
			dwTextFormat->GetFontStretch(),
			dwTextFormat->GetFontStyle(),
			&dwFont.getAssign()
		);

		DWRITE_FONT_METRICS fontMetrics;
		dwFont->GetMetrics(&fontMetrics);

		outAscent = (int32_t)(dwTextFormat->GetFontSize() * fontMetrics.ascent / fontMetrics.designUnitsPerEm + 0.5f);
		outDescent = (int32_t)(dwTextFormat->GetFontSize() * fontMetrics.descent / fontMetrics.designUnitsPerEm + 0.5f);
	}
	else	// Inside begin/end thus use current paint context.
	{
		return getAscentAndDescent(outAscent, outDescent);
	}
}

int32_t CanvasDirect2DWin32::getAdvance(Window& hWnd, const Font& font, wchar_t ch, wchar_t next) const
{
	if (!m_inPaint)
	{
		return getExtent(hWnd, font, std::wstring(1, ch)).cx;
	}
	else	// Inside begin/end thus use current paint context.
	{
		return getAdvance(ch, next);
	}
}

int32_t CanvasDirect2DWin32::getLineSpacing(Window& hWnd) const
{
	return 0;
}

Size CanvasDirect2DWin32::getExtent(Window& hWnd, const Font& font, const std::wstring& text) const
{
	if (!m_inPaint)
	{
		const int32_t dpi = hWnd.dpi();
		const int32_t fontSize = (font.getSize().get() * dpi) / 96.0f;

		ComRef< IDWriteTextFormat > dwTextFormat;
		s_dwFactory->CreateTextFormat(
			font.getFace().c_str(),
			NULL,
			font.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
			font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			L"",
			&dwTextFormat.getAssign()
		);
		if (!dwTextFormat)
			return Size(0, 0);

		dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
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
			dwtm.widthIncludingTrailingWhitespace,
			dwtm.height
		);
	}
	else	// Inside begin/end thus use current paint context.
	{
		return getExtent(text);
	}
}

void CanvasDirect2DWin32::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	if (realizeFont())
	{
		outAscent = (int32_t)(m_dwTextFormat->GetFontSize() * m_fontMetrics.ascent / m_fontMetrics.designUnitsPerEm + 0.5f);
		outDescent = (int32_t)(m_dwTextFormat->GetFontSize() * m_fontMetrics.descent / m_fontMetrics.designUnitsPerEm + 0.5f);
	}
	else
	{
		outAscent =
		outDescent = 0;
	}
}

int32_t CanvasDirect2DWin32::getAdvance(wchar_t ch, wchar_t next) const
{
	return getExtent(std::wstring(1, ch)).cx;
}

int32_t CanvasDirect2DWin32::getLineSpacing() const
{
	return 0;
}

Size CanvasDirect2DWin32::getExtent(const std::wstring& text) const
{
	if (!realizeFont())
		return Size(0, 0);

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
		dwtm.widthIncludingTrailingWhitespace,
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
	if (font == m_font)
		return;

	m_font = font;

	// Release previous font, the actual font is created later when text
	// is first drawn using the new font.
	m_dwFont.release();
	m_dwTextFormat.release();
}

const IFontMetric* CanvasDirect2DWin32::getFontMetric() const
{
	return this;
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
	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_d2dRenderTarget->DrawLine(
		D2D1::Point2F(x1 + 0.5f, y1 + 0.5f),
		D2D1::Point2F(x2 + 0.5f, y2 + 0.5f),
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
		D2D1::Point2F(pnts[0].x + 0.5f, pnts[0].y + 0.5f),
		D2D1_FIGURE_BEGIN_HOLLOW
	);

	for (int i = 1; i < npnts; ++i)
	{
		d2dGeometrySink->AddLine(
			D2D1::Point2F(pnts[i].x + 0.5f, pnts[i].y + 0.5f)
		);
	}

	d2dGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
	d2dGeometrySink->Close();

	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_d2dRenderTarget->DrawGeometry(
		d2dPathGeometry,
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::drawCurve(const Point& start, const Point& control, const Point& end)
{
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
		D2D1::Point2F(start.x + 0.5f, start.y + 0.5f),
		D2D1_FIGURE_BEGIN_HOLLOW
	);

	D2D1_BEZIER_SEGMENT segment;
	segment.point1 = D2D1::Point2F(start.x + 0.5f, start.y + 0.5f);
	segment.point2 = D2D1::Point2F(control.x + 0.5f, control.y + 0.5f);
	segment.point3 = D2D1::Point2F(end.x + 0.5f, end.y + 0.5f);
	d2dGeometrySink->AddBezier(segment);

	d2dGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
	d2dGeometrySink->Close();

	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_d2dRenderTarget->DrawGeometry(
		d2dPathGeometry,
		m_d2dForegroundBrush,
		m_strokeWidth
	);
}

void CanvasDirect2DWin32::fillCircle(int x, int y, float radius)
{
	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_d2dRenderTarget->FillEllipse(
		D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
		m_d2dBackgroundBrush
	);
}

void CanvasDirect2DWin32::drawCircle(int x, int y, float radius)
{
	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
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
	const Rect rc2 = rc.getUnified();
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
		D2D1::RectF(rc2.left + 0.5f, rc2.top + 0.5f, rc2.right - 0.5f, rc2.bottom - 0.5f),
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
			D2D1::RectF(rc2.left + 0.5f, rc2.top + 0.5f, rc2.right - 0.5f, rc2.bottom - 0.5f),
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

	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_d2dRenderTarget->DrawGeometry(
		d2dPathGeometry,
		m_d2dForegroundBrush,
		m_strokeWidth
	);
	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
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
			D2D1::Point2F(pnts[i].x + 0.5f, pnts[i].y + 0.5f)
		);
	}

	d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
	d2dGeometrySink->Close();

	m_d2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_d2dRenderTarget->FillGeometry(
		d2dPathGeometry,
		m_d2dBackgroundBrush
	);
}

void CanvasDirect2DWin32::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter)
{
	drawBitmap(dstAt, size, srcAt, size, bitmap, blendMode, filter);
}

DEFINE_GUID(CLSID_D2D1Tint, 0x36312b17, 0xf7dd, 0x4014, 0x91, 0x5d, 0xff, 0xca, 0x76, 0x8c, 0xf2, 0x11);

void CanvasDirect2DWin32::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter)
{
	ID2D1Bitmap* bm = getCachedBitmap(bitmap);
	if (!bm)
		return;

	if (blendMode == BlendMode::Modulate)
	{
		//ID2D1DeviceContext* deviceContext;
		//m_d2dRenderTarget->QueryInterface(&deviceContext);

		//ID2D1Effect* tintEffect;
		//deviceContext->CreateEffect(CLSID_D2D1Tint, &tintEffect);

		m_d2dTintEffect->SetInput(0, bm);
		m_d2dTintEffect->SetValue(D2D1_TINT_PROP_COLOR, m_gradientStops[1].color);
		m_d2dTintEffect->SetValue(D2D1_TINT_PROP_CLAMP_OUTPUT, TRUE);

		m_d2dDeviceContext->DrawImage(
			m_d2dTintEffect,
			D2D1::Point2F(dstAt.x, dstAt.y),
			D2D1::RectF(
				srcAt.x, srcAt.y,
				srcAt.x + srcSize.cx, srcAt.y + srcSize.cy
			),
			D2D1_INTERPOLATION_MODE_LINEAR
		);

		//tintEffect->Release();
		//deviceContext->Release();
	}
	else
	{
		D2D1_BITMAP_INTERPOLATION_MODE im = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
		if (filter == Filter::Nearest)
			im = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
		else if (dstSize.cx == srcSize.cx && dstSize.cy == srcSize.cy)
			im = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

		m_d2dRenderTarget->DrawBitmap(
			bm,
			D2D1::RectF(
				dstAt.x, dstAt.y,
				dstAt.x + dstSize.cx, dstAt.y + dstSize.cy
			),
			1.0f,
			im,
			D2D1::RectF(
				srcAt.x, srcAt.y,
				srcAt.x + srcSize.cx, srcAt.y + srcSize.cy
			)
		);
	}
}

void CanvasDirect2DWin32::drawText(const Point& at, const std::wstring& text)
{
	if (text.empty() || !realizeFont())
		return;

	if (text.length() > 1)
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
			return;

		if (m_font.isUnderline())
		{
			DWRITE_TEXT_RANGE range;
			range.startPosition = 0;
			range.length = text.length();
			dwLayout->SetUnderline(TRUE, range);
		}

		// Remove line gap; it's being added on top of ascent.
		const int32_t lineGap = m_dwTextFormat->GetFontSize() * m_fontMetrics.lineGap / m_fontMetrics.designUnitsPerEm;

		m_d2dRenderTarget->DrawTextLayout(
			D2D1::Point2F(at.x, at.y - lineGap),
			dwLayout,
			m_d2dForegroundBrush
		);
	}
	else if (text.length() == 1)
	{
		// Optimize for single characters since RichEdit draw each glyph one by one.
		const int32_t fontSize = (int32_t)((m_font.getSize().get() * m_dpi) / 96.0f);

		UINT32 codePoint = (UINT32)text[0];
		UINT16 glyphIndex = 0;
		HRESULT hr = m_dwFontFace->GetGlyphIndices(&codePoint, 1, &glyphIndex);
		if (!SUCCEEDED(hr))
			return;

		FLOAT glyphAdvance = 0.0f;

		DWRITE_GLYPH_OFFSET glyphOffset = {};
		glyphOffset.advanceOffset == 0.0f;
		glyphOffset.ascenderOffset = -fontSize;

		DWRITE_GLYPH_RUN grun = {};
		grun.fontFace = m_dwFontFace;
		grun.fontEmSize = fontSize;
		grun.glyphCount = 1;
		grun.glyphIndices = &glyphIndex;
		grun.glyphAdvances = &glyphAdvance;
		grun.glyphOffsets = &glyphOffset;
		grun.isSideways = FALSE;
		grun.bidiLevel = 0;

		// Remove line gap; it's being added on top of ascent.
		const int32_t lineGap = m_dwTextFormat->GetFontSize() * m_fontMetrics.lineGap / m_fontMetrics.designUnitsPerEm;

		m_d2dRenderTarget->DrawGlyphRun(
			D2D1::Point2F(at.x, at.y - lineGap),
			&grun,
			m_d2dForegroundBrush
		);
	}
}

void* CanvasDirect2DWin32::getSystemHandle()
{
	return NULL;
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

ID2D1Bitmap* CanvasDirect2DWin32::getCachedBitmap(const ISystemBitmap* bm)
{
	const BitmapWin32* bmw32 = reinterpret_cast< const BitmapWin32* >(bm);

	auto it = m_cachedBitmaps.find(bmw32->getTag());
	if (it != m_cachedBitmaps.end())
	{
		if (it->second.revision == bmw32->getRevision())
			return it->second.bitmap;
		else
			m_cachedBitmaps.erase(it);
	}

	const Size size = bmw32->getSize();

	const uint32_t* colorBits = (const uint32_t*)(bmw32->haveAlpha() ? bmw32->getBitsPreMulAlpha() : bmw32->getBits());
	AutoArrayPtr< uint32_t > bits(new uint32_t [size.cx * size.cy]);

	for (uint32_t y = 0; y < size.cy; ++y)
	{
		const uint32_t srcOffset = (size.cy - y - 1) * size.cx;
		const uint32_t dstOffset = y * size.cx;
		for (int32_t x = 0; x < size.cx; ++x)
			bits[dstOffset + x] = colorBits[srcOffset + x];
	}

	const D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, bmw32->haveAlpha() ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
	const D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(pixelFormat);

	ComRef< ID2D1Bitmap > d2dBitmap;
	HRESULT hr = m_d2dRenderTarget->CreateBitmap(
		D2D1::SizeU(size.cx, size.cy),
		bits.c_ptr(),
		size.cx * 4,
		bitmapProps,
		&d2dBitmap.getAssign()
	);
	if (FAILED(hr))
		return nullptr;

	m_cachedBitmaps[bmw32->getTag()] = { bmw32->getRevision(), d2dBitmap };
	return d2dBitmap;
}

void CanvasDirect2DWin32::flushCachedBitmaps()
{
	m_cachedBitmaps.clear();
}

bool CanvasDirect2DWin32::realizeFont() const
{
	if (m_dwFont)
		return true;

	const int32_t fontSize = (int32_t)((m_font.getSize().get() * m_dpi) / 96.0f);

	auto it = m_cachedFonts.find(std::make_pair(m_font, fontSize));
	if (it != m_cachedFonts.end())
	{
		m_dwTextFormat = it->second.dwTextFormat;
		m_dwFont = it->second.dwFont;
		return true;
	}

	s_dwFactory->CreateTextFormat(
		m_font.getFace().c_str(),
		NULL,
		m_font.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
		m_font.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"",
		&m_dwTextFormat.getAssign()
	);
	if (!m_dwTextFormat)
		return false;

	m_dwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	m_dwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_dwTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	ComRef< IDWriteFontCollection > collection;
	m_dwTextFormat->GetFontCollection(&collection.getAssign());

	UINT32 findex;
	BOOL exists;
	collection->FindFamilyName(m_font.getFace().c_str(), &findex, &exists);

	if (exists)
	{
		ComRef< IDWriteFontFamily > ffamily;
		collection->GetFontFamily(findex, &ffamily.getAssign());
		T_FATAL_ASSERT(ffamily != nullptr);

		ffamily->GetFirstMatchingFont(
			m_dwTextFormat->GetFontWeight(),
			m_dwTextFormat->GetFontStretch(),
			m_dwTextFormat->GetFontStyle(),
			&m_dwFont.getAssign()
		);

		if (m_dwFont != nullptr)
			m_dwFont->GetMetrics(&m_fontMetrics);
	}

	if (m_dwFont == nullptr)
		return false;

	m_dwFont->CreateFontFace(&m_dwFontFace.getAssign());
	T_FATAL_ASSERT(m_dwFontFace != nullptr);

	m_cachedFonts.insert(
		std::make_pair(m_font, fontSize),
		{
			m_dwTextFormat,
			m_dwFont,
			m_dwFontFace
		}
	);

	return true;
}

}

#endif
