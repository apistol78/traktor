/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Ui/FontMetric.h"
#include "Ui/Itf/ICanvas.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;
class Widget;

/*! Canvas
 * \ingroup UI
 */
class T_DLLCLASS Canvas : public Object
{
	T_RTTI_CLASS;

public:
	explicit Canvas(ICanvas* canvas, Widget* widget);

	void setForeground(const Color4ub& foreground);

	const Color4ub& getForeground() const;

	void setBackground(const Color4ub& background);

	const Color4ub& getBackground() const;

	void setFont(const Font& font);

	FontMetric getFontMetric() const;

	void setLineStyle(LineStyle lineStyle);

	void setPenThickness(int thickness);

	void setClipRect(const Rect& rc);

	void resetClipRect();

	void drawPixel(int x, int y);

	void drawPixel(const Point& p);

	void drawPixel(int x, int y, const Color4ub& c);

	void drawPixel(const Point& p, const Color4ub& c);

	void drawLine(int x1, int y1, int x2, int y2);

	void drawLine(const Point& start, const Point& end);

	void drawLines(const std::vector< Point >& pnts);

	void drawLines(const Point* pnts, int npnts);

	void drawCurve(const Point& start, const Point& control, const Point& end);

	void fillCircle(const Point& p, float radius);

	void drawCircle(const Point& p, float radius);

	void drawEllipticArc(const Point& p, const Size& s, float start, float end);

	void drawSpline(const std::vector< Point >& pnts);

	void drawSpline(const Point* pnts, int npnts);

	void fillRect(const Rect& rc);

	void fillGradientRect(const Rect& rc, bool vertical = true);

	void drawRect(const Rect& rc);

	void drawRoundRect(const Rect& rc, int radius);

	void drawPolygon(const Point* pnts, int count);

	void fillPolygon(const Point* pnts, int count);

	void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, BlendMode blendMode = BlendMode::Opaque, Filter filter = Filter::Linear);

	void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, BlendMode blendMode = BlendMode::Opaque, Filter filter = Filter::Linear);

	void drawText(const Point& at, const std::wstring& text);

	void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop);

	void drawGlyph(const Point& at, const wchar_t chr);

	ICanvas* getICanvas() const { return m_canvas; }

private:
	ICanvas* m_canvas;
	Widget* m_widget;
	Color4ub m_foreground;
	Color4ub m_background;
};

}
