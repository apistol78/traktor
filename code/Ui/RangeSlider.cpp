/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/RangeSlider.h"

#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
namespace
{

const Unit c_margin = 8_ut;
const Unit c_knobRadius = 6_ut;
const Unit c_trackHeight = 4_ut;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.RangeSlider", RangeSlider, Widget)

bool RangeSlider::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	addEventHandler< MouseTrackEvent >(this, &RangeSlider::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &RangeSlider::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &RangeSlider::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &RangeSlider::eventMouseMove);
	addEventHandler< PaintEvent >(this, &RangeSlider::eventPaint);

	return true;
}

void RangeSlider::setLimits(const Range< int32_t >& limits)
{
	m_limits = limits.canonical();
	update();
}

void RangeSlider::setRange(const Range< int32_t >& range)
{
	m_range = range.canonical();
	update();
}

const Range< int32_t >& RangeSlider::getRange() const
{
	return m_range;
}

Size RangeSlider::getPreferredSize(const Size& hint) const
{
	return Size(pixel(100_ut), pixel(20_ut));
}

Size RangeSlider::getMaximumSize() const
{
	return Size(65535, pixel(20_ut));
}

void RangeSlider::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable(true))
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	const int32_t value[2] = { m_limits.clamp(m_range.min), m_limits.clamp(m_range.max) };
	const int32_t dist = sz.cx - pixel(c_margin) * 2;

	for (int32_t i = 0; i < 2; ++i)
	{
		const int32_t knob = pixel(c_margin) + int32_t(dist * float(value[i] - m_limits.min) / m_limits.delta());
		const int32_t knobL = knob - pixel(c_knobRadius);
		const int32_t knobR = knob + pixel(c_knobRadius);

		if (pt.x >= knobL && pt.x <= knobR)
		{
			setCapture();

			m_drag = i;
			update();

			break;
		}
	}
}

void RangeSlider::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void RangeSlider::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_drag < 0)
		return;

	releaseCapture();

	m_drag = -1;
	update();
}

void RangeSlider::eventMouseMove(MouseMoveEvent* event)
{
	if (m_drag < 0)
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	const int32_t x = pt.x - pixel(c_margin);
	const int32_t dist = sz.cx - pixel(c_margin) * 2;
	const int32_t value = int32_t(m_limits.delta() * float(x) / dist + 0.5f);

	m_range[m_drag] = m_limits.clamp(value);
	update();

	ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

void RangeSlider::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	const bool enabled = isEnable(true);
	const int32_t cy = rcInner.getCenter().y;
	const int32_t half = pixel(c_trackHeight) / 2;
	const int32_t radius = pixel(c_knobRadius);

	// Track, as a rounded bar the full width.
	const Rect rcTrack(
		rcInner.left + pixel(c_margin),
		cy - half,
		rcInner.right - pixel(c_margin),
		cy + half);
	canvas.setBackground(ss->getColor(this, L"track-color"));
	canvas.fillRoundRect(rcTrack, half);

	const int32_t dist = rcInner.getSize().cx - pixel(c_margin) * 2;
	const int32_t valueL = m_limits.clamp(m_range.min);
	const int32_t knobL = rcInner.left + pixel(c_margin) + int32_t(dist * float(valueL - m_limits.min) / m_limits.delta());
	const int32_t valueR = m_limits.clamp(m_range.max);
	const int32_t knobR = rcInner.left + pixel(c_margin) + int32_t(dist * float(valueR - m_limits.min) / m_limits.delta());

	if (enabled)
	{
		const Rect rcActive(knobL, rcTrack.top, knobR, rcTrack.bottom);
		canvas.setBackground(ss->getColor(this, L"track-color-active"));
		canvas.fillRect(rcActive);
	}

	for (int32_t i = 0; i < 2; ++i)
	{
		const int32_t value = m_limits.clamp(m_range[i]);
		const int32_t knob = rcInner.left + pixel(c_margin) + int32_t(dist * float(value - m_limits.min) / m_limits.delta());

		const wchar_t* knobColor = L"knob-color";
		if (!enabled)
			knobColor = L"knob-color-disabled";
		else if (m_drag || m_hover)
			knobColor = L"knob-color-hover";

		canvas.setBackground(ss->getColor(this, knobColor));
		canvas.fillCircle(Point(knob, cy), (float)radius);

		canvas.setForeground(ss->getColor(this, L"knob-border-color"));
		canvas.drawCircle(Point(knob, cy), (float)radius);
	}

	event->consume();
}

}
