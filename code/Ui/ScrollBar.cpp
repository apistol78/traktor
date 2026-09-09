/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/ScrollBar.h"

namespace traktor::ui
{
	namespace
	{

int32_t getPrimaryPosition(const Point& p, bool vertical)
{
	return vertical ? p.y : p.x;
}

int32_t getPrimarySize(const Size& s, bool vertical)
{
	return vertical ? s.cy : s.cx;
}

const Unit c_sliderInset = 4_ut;
const Unit c_sliderMinimum = 60_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ScrollBar", ScrollBar, Widget)

bool ScrollBar::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	m_vertical = bool((style & WsVertical) == WsVertical);

	addEventHandler< MouseTrackEvent >(this, &ScrollBar::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &ScrollBar::eventMouseButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ScrollBar::eventMouseButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ScrollBar::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ScrollBar::eventPaint);

	return true;
}

void ScrollBar::setRange(int32_t range)
{
	m_range = max(range, 0);
	m_position = clamp(m_position, 0, m_range - (m_page - 1));
	update();
}

int32_t ScrollBar::getRange() const
{
	return m_range;
}

void ScrollBar::setPage(int32_t page)
{
	m_page = max(page, 1);
	update();
}

int32_t ScrollBar::getPage() const
{
	return m_page;
}

void ScrollBar::setPosition(int32_t position)
{
	m_position = clamp(position, 0, m_range - (m_page - 1));
	update();
}

int32_t ScrollBar::getPosition() const
{
	return m_position;
}

void ScrollBar::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void ScrollBar::eventMouseButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	const Rect rcInner = getInnerRect();

	Rect rcSlider;
	if (!getSliderRect(rcInner, rcSlider))
		return;

	const int32_t at = getPrimaryPosition(event->getPosition(), m_vertical);
	const int32_t sliderBegin = getPrimaryPosition(rcSlider.getTopLeft(), m_vertical);
	const int32_t sliderEnd = getPrimaryPosition(rcSlider.getBottomRight(), m_vertical);

	if (at >= sliderBegin && at <= sliderEnd)
	{
		m_trackOffset = at - sliderBegin;
		setCapture();
		return;
	}

	if (at < sliderBegin)
		m_position = max(m_position - (m_page - 1), 0);
	else
		m_position = min(m_position + m_page, m_range - (m_page - 1));

	ScrollEvent scrollEvent(this, m_position);
	raiseEvent(&scrollEvent);
	update();
}

void ScrollBar::eventMouseButtonUp(MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	releaseCapture();
}

void ScrollBar::eventMouseMove(MouseMoveEvent* event)
{
	if (hasCapture())
	{
		const Rect rcInner = getInnerRect();

		Rect rcSlider;
		if (getSliderRect(rcInner, rcSlider))
		{
			const int32_t inset = pixel(c_sliderInset);
			const int32_t range = getPrimarySize(rcInner.getSize(), m_vertical) - inset * 2;
			const int32_t length = getPrimarySize(rcSlider.getSize(), m_vertical);
			const int32_t travel = range - length;
			const int32_t scrollable = m_range - (m_page - 1);

			// A slider filling its track has nowhere to travel.
			if (travel > 0 && scrollable > 0)
			{
				const int32_t position0 = m_position;
				const int32_t at = getPrimaryPosition(event->getPosition(), m_vertical);

				m_position = (at - getPrimaryPosition(rcInner.getTopLeft(), m_vertical) - inset - m_trackOffset) * scrollable / travel;
				m_position = clamp(m_position, 0, scrollable);

				if (m_position != position0)
				{
					ScrollEvent scrollEvent(this, m_position);
					raiseEvent(&scrollEvent);
				}
			}
		}
	}
	update();
}

void ScrollBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	Rect rcSlider;
	if (getSliderRect(rcInner, rcSlider))
	{
		const bool hover = isEnable(true) && m_hover && rcSlider.inside(getMousePosition());
		const int32_t radius = getPrimarySize(rcSlider.getSize(), !m_vertical) / 2;

		canvas.setBackground(ss->getColor(this, hover ? L"color-slider-hover" : L"color-slider"));
		canvas.fillRoundRect(rcSlider, radius);
	}

	event->consume();
}

Size ScrollBar::getPreferredSize(const Size& hint) const
{
	return Size(
		pixel(16_ut),
		pixel(16_ut)
	);
}

Size ScrollBar::getMaximumSize() const
{
	if (m_vertical)
		return Size(pixel(16_ut), 65535);
	else
		return Size(65535, pixel(16_ut));
}


bool ScrollBar::getSliderRect(const Rect& rcInner, Rect& outSlider) const
{
	if (m_range <= 0)
		return false;

	const int32_t inset = pixel(c_sliderInset);

	const int32_t range = getPrimarySize(rcInner.getSize(), m_vertical) - inset * 2;
	if (range <= 0)
		return false;

	const int32_t length = min(max(m_page * range / m_range, pixel(c_sliderMinimum)), range);
	const int32_t travel = range - length;
	const int32_t scrollable = m_range - (m_page - 1);
	const int32_t offset = (travel > 0 && scrollable > 0) ? m_position * travel / scrollable : 0;

	if (m_vertical)
		outSlider = Rect(
			rcInner.left + inset,
			rcInner.top + inset + offset,
			rcInner.right - inset,
			rcInner.top + inset + offset + length
		);
	else
		outSlider = Rect(
			rcInner.left + inset + offset,
			rcInner.top + inset,
			rcInner.left + inset + offset + length,
			rcInner.bottom - inset
		);

	return true;
}

}
