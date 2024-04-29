/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Const.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/Graph/GraphCanvas.h"

namespace traktor::ui
{
	namespace
	{

int32_t round(float f)
{
	if (f > -0.5f)
		return (int32_t)std::floor(f + 0.5f);
	else if (f < -0.5f)
		return -(int32_t)std::floor(-f + 0.5f);
	else
		return 0;
}

Size operator * (const Size& sz, float scale)
{
	return Size(
		round(sz.cx * scale),
		round(sz.cy * scale)
	);
}

Size operator / (const Size& sz, float scale)
{
	return sz * (1.0f / scale);
}

Point operator * (const Point& pt, float scale)
{
	return Point(
		round(pt.x * scale),
		round(pt.y * scale)
	);
}

Rect operator * (const Rect& rc, float scale)
{
	return Rect(
		rc.getTopLeft() * scale,
		rc.getBottomRight() * scale
	);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GraphCanvas", GraphCanvas, Object)

GraphCanvas::GraphCanvas(Widget* owner, Canvas* canvas, const PaintSettings& paintSettings, float scale)
:	m_owner(owner)
,	m_canvas(canvas)
,	m_paintSettings(paintSettings)
,	m_scale(scale)
{
}

void GraphCanvas::setForeground(const Color4ub& foreground)
{
	m_canvas->setForeground(foreground);
}

void GraphCanvas::setBackground(const Color4ub& background)
{
	m_canvas->setBackground(background);
}

void GraphCanvas::setFont(const Font& font)
{
	m_originalFont = font;

	m_scaledFont = font;
	m_scaledFont.setSize(
		Unit((int32_t)std::ceil(font.getSize().get() * m_scale))
	);

	m_canvas->setFont(m_scaledFont);
}

void GraphCanvas::drawLine(const Point& start, const Point& end, int32_t thickness)
{
	m_canvas->setPenThickness((int32_t)std::ceil(thickness * m_scale));
	m_canvas->drawLine(
		start * m_scale,
		end * m_scale
	);
	m_canvas->setPenThickness(1);
}

void GraphCanvas::drawLines(const AlignedVector< Point >& pnts, int32_t thickness)
{
	std::vector< Point > tpnts;
	tpnts.reserve(pnts.size());

	for (const auto& pnt : pnts)
		tpnts.push_back(pnt * m_scale);

	m_canvas->setPenThickness((int32_t)std::ceil(thickness * m_scale));
	m_canvas->drawLines(tpnts);
	m_canvas->setPenThickness(1);
}

void GraphCanvas::drawCurve(const Point& start, const Point& control, const Point& end, int32_t thickness)
{
	m_canvas->setPenThickness((int32_t)std::ceil(thickness * m_scale));
	m_canvas->drawCurve(start * m_scale, control * m_scale, end * m_scale);
	m_canvas->setPenThickness(1);
}

void GraphCanvas::drawRect(const Rect& rc)
{
	m_canvas->drawRect(rc * m_scale);
}

void GraphCanvas::fillRect(const Rect& rc)
{
	m_canvas->fillRect(rc * m_scale);
}

void GraphCanvas::fillPolygon(const Point* pnts, int count)
{
	std::vector< Point > tpnts; tpnts.reserve(count);
	for (int i = 0; i < count; ++i)
		tpnts.push_back(pnts[i] * m_scale);
	m_canvas->fillPolygon(&tpnts[0], count);
}

void GraphCanvas::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, BlendMode blendMode)
{
	const Point sdstAt(
		(int32_t)std::floor(dstAt.x * m_scale),
		(int32_t)std::floor(dstAt.y * m_scale)
	);
	const Size sdstSize(
		(int32_t)std::floor(dstSize.cx * m_scale + 1.0f),
		(int32_t)std::floor(dstSize.cy * m_scale + 1.0f)
	);
	m_canvas->drawBitmap(
		sdstAt,
		sdstSize,
		srcAt,
		srcSize,
		bitmap,
		blendMode
	);
}

void GraphCanvas::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	if (m_scaledFont.getSize() < 4_ut)
		return;

	m_canvas->drawText(
		rc * m_scale,
		text,
		halign,
		valign
	);
}

Size GraphCanvas::getTextExtent(const std::wstring& text) const
{
	return m_canvas->getFontMetric().getExtent(text) / m_scale;
}

void GraphCanvas::draw9gridBitmap(const Point& dstAt, const Size& dstSize, IBitmap* bitmap, BlendMode blendMode)
{
	const Size sz = bitmap->getSize(m_owner);

	const Point sdstAt(
		(int32_t)std::floor(dstAt.x * m_scale),
		(int32_t)std::floor(dstAt.y * m_scale)
	);
	const Size sdstSize(
		(int32_t)std::floor(dstSize.cx * m_scale + 1.0f),
		(int32_t)std::floor(dstSize.cy * m_scale + 1.0f)
	);

	const int32_t tw = sz.cx / 3;
	const int32_t th = sz.cy / 3;

	const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
	const int32_t sy[] = { 0, th, sz.cy - th, sz.cy };

	const int32_t dw = sdstSize.cx;
	const int32_t dh = sdstSize.cy;

	const int32_t stw = (int32_t)std::floor(tw * m_scale);
	const int32_t sth = (int32_t)std::floor(th * m_scale);

	const int32_t dx[] = { 0, stw, dw - stw, dw };
	const int32_t dy[] = { 0, sth, dh - sth, dh };

	for (int32_t iy = 0; iy < 3; ++iy)
	{
		for (int32_t ix = 0; ix < 3; ++ix)
		{
			m_canvas->drawBitmap(
				sdstAt + Size(dx[ix], dy[iy]),
				Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
				Point(sx[ix], sy[iy]),
				Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
				bitmap,
				BlendMode::Alpha
			);
		}
	}
}

}
