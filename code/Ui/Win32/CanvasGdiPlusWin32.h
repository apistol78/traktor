/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined(T_USE_GDI_PLUS)
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#include <windows.h>
// \hack As we don't want min/max defined but gdiplus.h require them
// we temporarily define them first.
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#include <gdiplus.h>
#undef min
#undef max
#include "Core/Misc/AutoPtr.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Win32/CanvasWin32.h"

namespace traktor::ui
{

/*!
 * \ingroup UIW32
 */
class CanvasGdiPlusWin32 : public CanvasWin32
{
public:
	CanvasGdiPlusWin32();

	virtual ~CanvasGdiPlusWin32();

	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC) override final;

	virtual void endPaint(Window& hWnd) override final;

	virtual void getAscentAndDescent(Window& hWnd, int32_t& outAscent, int32_t& outDescent) const override final;

	virtual int32_t getAdvance(Window& hWnd, wchar_t ch, wchar_t next) const override final;

	virtual int32_t getLineSpacing(Window& hWnd) const override final;

	virtual Size getExtent(Window& hWnd, const std::wstring& text) const override final;

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

	virtual void* getSystemHandle() override final;

	static bool startup();

	static void shutdown();

private:
	PAINTSTRUCT m_ps;
	static HDC ms_hGlobalDC;
	static AutoPtr< Gdiplus::Graphics > ms_globalGraphics;
	HDC m_hDC;
	HFONT m_hFont;
	bool m_ownDC;
	bool m_doubleBuffer;
	int32_t m_dpi;
	uint32_t m_offScreenBitmapWidth;
	uint32_t m_offScreenBitmapHeight;
	AutoPtr< Gdiplus::Bitmap > m_offScreenBitmap;
	mutable AutoPtr< Gdiplus::Graphics > m_graphics;
	mutable AutoPtr< Gdiplus::Font > m_font;
	Gdiplus::Color m_foreGround;
	Gdiplus::Color m_backGround;
	Gdiplus::Pen m_pen;
	Gdiplus::SolidBrush m_brush;
	SmallMap< int32_t, Gdiplus::Bitmap* > m_cachedBitmaps;

	Gdiplus::Bitmap* getCachedBitmap(const ISystemBitmap* bm);

	void flushCachedBitmaps();
};

}

#endif	// T_USE_GDI_PLUS
