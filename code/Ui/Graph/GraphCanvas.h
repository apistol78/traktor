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
#include "Core/Containers/AlignedVector.h"
#include "Ui/Enums.h"
#include "Ui/Font.h"
#include "Ui/Rect.h"

namespace traktor
{

class Color4ub;

}

namespace traktor::ui
{

class Canvas;
class IBitmap;
class PaintSettings;

class GraphCanvas : public Object
{
	T_RTTI_CLASS;

public:
	explicit GraphCanvas(Widget* owner, Canvas* canvas, const PaintSettings& paintSettings, float scale);

	void setForeground(const Color4ub& foreground);

	void setBackground(const Color4ub& background);

	void setFont(const Font& font);

	void drawLine(const Point& start, const Point& end, int32_t thickness);

	void drawLines(const AlignedVector< Point >& pnts, int32_t thickness);

	void drawCurve(const Point& start, const Point& control, const Point& end, int32_t thickness);

	void drawRect(const Rect& rc);

	void fillRect(const Rect& rc);

	void fillPolygon(const Point* pnts, int count);

	void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, BlendMode blendMode);

	void drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign);

	Size getTextExtent(const std::wstring& text) const;

	const PaintSettings& getPaintSettings() const { return m_paintSettings; }

	void draw9gridBitmap(const Point& dstAt, const Size& dstSize, IBitmap* bitmap, BlendMode blendMode);

private:
	Widget* m_owner;
	Canvas* m_canvas;
	const PaintSettings& m_paintSettings;
	float m_scale;
	Font m_originalFont;
	Font m_scaledFont;
};

}
