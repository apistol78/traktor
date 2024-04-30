/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined(T_USE_DIRECT2D)
#include <d2d1_1.h>
#include <d2d1effects_2.h>
#include <dwrite.h>
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/ComRef.h"
#include "Ui/Win32/CanvasWin32.h"

namespace traktor::ui
{

/*!
 * \ingroup UIW32
 */
class CanvasDirect2DWin32 : public CanvasWin32
{
public:
	CanvasDirect2DWin32();

	virtual ~CanvasDirect2DWin32();

	// CanvasWin32

	virtual bool beginPaint(Window& hWnd, const Font& font, bool doubleBuffer, HDC hDC) override final;

	virtual void endPaint(Window& hWnd) override final;

	virtual void getAscentAndDescent(Window& hWnd, const Font& font, int32_t& outAscent, int32_t& outDescent) const override final;

	virtual int32_t getAdvance(Window& hWnd, const Font& font, wchar_t ch, wchar_t next) const override final;

	virtual int32_t getLineSpacing(Window& hWnd) const override final;

	virtual Size getExtent(Window& hWnd, const Font& font, const std::wstring& text) const override final;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override final;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override final;

	virtual int32_t getLineSpacing() const override final;

	virtual Size getExtent(const std::wstring& text) const override final;

	// ICanvas

	virtual void setForeground(const Color4ub& foreground) override final;

	virtual void setBackground(const Color4ub& background) override final;

	virtual void setFont(const Font& font) override final;

	virtual const IFontMetric* getFontMetric() const override final;

	virtual void setLineStyle(LineStyle lineStyle) override final;

	virtual void setPenThickness(int thickness) override final;

	virtual void setClipRect(const Rect& rc) override final;

	virtual void resetClipRect() override final;

	virtual void drawPixel(int x, int y, const Color4ub& c) override final;

	virtual void drawLine(int x1, int y1, int x2, int y2) override final;

	virtual void drawLines(const Point* pnts, int npnts) override final;

	virtual void drawCurve(const Point& start, const Point& control, const Point& end) override final;

	virtual void fillCircle(int x, int y, float radius) override final;

	virtual void drawCircle(int x, int y, float radius) override final;

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end) override final;

	virtual void drawSpline(const Point* pnts, int npnts) override final;

	virtual void fillRect(const Rect& rc) override final;

	virtual void fillGradientRect(const Rect& rc, bool vertical = true) override final;

	virtual void drawRect(const Rect& rc) override final;

	virtual void drawRoundRect(const Rect& rc, int radius) override final;

	virtual void drawPolygon(const Point* pnts, int count) override final;

	virtual void fillPolygon(const Point* pnts, int count) override final;

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter) override final;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter) override final;

	virtual void drawText(const Point& at, const std::wstring& text) override final;

	virtual void drawGlyph(const Point& at, const wchar_t chr) override final;

	virtual void* getSystemHandle();

	static bool startup();

	static void shutdown();

private:
	struct CachedBitmap
	{
		int32_t revision;
		ComRef< ID2D1Bitmap > bitmap;
	};

	struct CachedFont
	{
		ComRef< IDWriteTextFormat > dwTextFormat;
		ComRef< IDWriteFont > dwFont;
		ComRef< IDWriteFontFace > dwFontFace;
	};

	bool m_inPaint = false;
	int32_t m_dpi = 0;
	ComRef< ID2D1DeviceContext > m_d2dDeviceContext;
	ComRef< ID2D1HwndRenderTarget > m_d2dRenderTarget;
	ComRef< ID2D1Effect > m_d2dTintEffect;
	ComRef< ID2D1SolidColorBrush > m_d2dForegroundBrush;
	ComRef< ID2D1SolidColorBrush > m_d2dBackgroundBrush;
	D2D1_GRADIENT_STOP m_gradientStops[2];
	ComRef< ID2D1GradientStopCollection > m_d2dGradientStops;
	mutable ComRef< IDWriteTextFormat > m_dwTextFormat;
	mutable ComRef< IDWriteFont > m_dwFont;
	mutable ComRef< IDWriteFontFace > m_dwFontFace;
	mutable DWRITE_FONT_METRICS m_fontMetrics;
	SmallMap< int32_t, CachedBitmap > m_cachedBitmaps;
	mutable SmallMap< std::pair< Font, int32_t >, CachedFont > m_cachedFonts;
	Font m_font;
	float m_strokeWidth = 1.0f;
	bool m_clip = false;

	mutable Font m_fontOffScreen;
	mutable int32_t m_fontOffScreenSize = -1;
	mutable ComRef< IDWriteTextFormat > m_dwTextFormatOffScreen;

	ID2D1Bitmap* getCachedBitmap(const ISystemBitmap* bm);

	void flushCachedBitmaps();

	bool realizeFont() const;
};

}

#endif	// T_USE_DIRECT2D

