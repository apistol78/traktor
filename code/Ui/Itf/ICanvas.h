/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <vector>
#include "Core/Math/Color4ub.h"
#include "Ui/Enums.h"
#include "Ui/Font.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

class IFontMetric;
class ISystemBitmap;

/*! Canvas interface.
 * \ingroup UI
 */
class ICanvas
{
public:
	virtual void setForeground(const Color4ub& foreground) = 0;

	virtual void setBackground(const Color4ub& background) = 0;

	virtual void setFont(const Font& font) = 0;

	virtual const IFontMetric* getFontMetric() const = 0;

	virtual void setLineStyle(LineStyle lineStyle) = 0;

	virtual void setPenThickness(int thickness) = 0;

	virtual void setClipRect(const Rect& rc) = 0;

	virtual void resetClipRect() = 0;

	virtual void drawPixel(int x, int y, const Color4ub& c) = 0;

	virtual void drawLine(int x1, int y1, int x2, int y2) = 0;

	virtual void drawLines(const Point* pnts, int npnts) = 0;

	virtual void drawCurve(const Point& start, const Point& control, const Point& end) = 0;

	virtual void fillCircle(int x, int y, float radius) = 0;

	virtual void drawCircle(int x, int y, float radius) = 0;

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end) = 0;

	virtual void drawSpline(const Point* pnts, int npnts) = 0;

	virtual void fillRect(const Rect& rc) = 0;

	virtual void fillGradientRect(const Rect& rc, bool vertical) = 0;

	virtual void drawRect(const Rect& rc) = 0;

	virtual void drawRoundRect(const Rect& rc, int radius) = 0;

	virtual void drawPolygon(const Point* pnts, int count) = 0;

	virtual void fillPolygon(const Point* pnts, int count) = 0;

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter) = 0;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, BlendMode blendMode, Filter filter) = 0;

	virtual void drawText(const Point& at, const std::wstring& text) = 0;

	virtual void drawGlyph(const Point& at, const wchar_t chr) = 0;

	virtual void* getSystemHandle() = 0;
};

	}
}

