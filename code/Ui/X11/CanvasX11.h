/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cairo.h>
#include "Ui/Itf/ICanvas.h"
#include "Ui/Itf/IFontMetric.h"

namespace traktor::ui
{

class CanvasX11
:	public ICanvas
,	public IFontMetric
{
public:
	explicit CanvasX11(cairo_t* cr, int32_t dpi);

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

	virtual void fillGradientRect(const Rect& rc, bool vertical) override final;

	virtual void drawRect(const Rect& rc) override final;

	virtual void drawRoundRect(const Rect& rc, int radius) override final;

	virtual void drawPolygon(const Point* pnts, int count) override final;

	virtual void fillPolygon(const Point* pnts, int count) override final;

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter) override final;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter) override final;

	virtual void drawText(const Point& at, const std::wstring& text) override final;

	virtual void drawGlyph(const Point& at, const wchar_t chr) override final;

	virtual void* getSystemHandle() override final;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override final;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override final;

	virtual int32_t getLineSpacing() const override final;

	virtual Size getExtent(const std::wstring& text) const override final;

private:
	cairo_t* m_cr;
	int32_t m_dpi;
	Color4ub m_currentSourceColor;
	Color4ub m_foreground;
	Color4ub m_background;
	int32_t m_thickness;
	Font m_font;
	mutable bool m_fontDirty;

	void setSourceColor(const Color4ub& color);

	bool realizeFont() const;
};

}
