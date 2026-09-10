/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Slider.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_margin = 8_ut;
const Unit c_knobRadius = 6_ut;
const Unit c_trackHeight = 4_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Slider", Slider, Widget)

bool Slider::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	addEventHandler< MouseTrackEvent >(this, &Slider::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &Slider::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Slider::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &Slider::eventMouseMove);
	addEventHandler< PaintEvent >(this, &Slider::eventPaint);

	return true;
}

void Slider::setRange(int32_t minValue, int32_t maxValue)
{
	m_range = Range< int32_t >(minValue, maxValue);
	update();
}

void Slider::setValue(int32_t value)
{
	m_value = value;
	update();
}

int32_t Slider::getValue() const
{
	return m_value;
}

Size Slider::getPreferredSize(const Size& hint) const
{
	return Size(pixel(100_ut), pixel(20_ut));
}

Size Slider::getMaximumSize() const
{
	return Size(65535, pixel(20_ut));
}

void Slider::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable(true))
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	const int32_t value = m_range.clamp(m_value);
	const int32_t dist = sz.cx - pixel(c_margin) * 2;
	const int32_t knob = pixel(c_margin) + int32_t(dist * float(value - m_range.min) / m_range.delta());
	const int32_t knobL = knob - pixel(c_knobRadius);
	const int32_t knobR = knob + pixel(c_knobRadius);

	if (pt.x >= knobL && pt.x <= knobR)
	{
		setCapture();

		m_drag = true;
		update();
	}
	else if (pt.x < knobL)
	{
		m_value = m_range.clamp(m_value - m_range.delta() / 10);
		update();

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
	else if (pt.x > knobR)
	{
		m_value = m_range.clamp(m_value + m_range.delta() / 10);
		update();

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
}

void Slider::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void Slider::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!m_drag)
		return;

	releaseCapture();

	m_drag = false;
	update();
}

void Slider::eventMouseMove(MouseMoveEvent* event)
{
	if (!m_drag)
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	const int32_t x = pt.x - pixel(c_margin);
	const int32_t dist = sz.cx - pixel(c_margin) * 2;
	const int32_t value = int32_t(m_range.delta() * float(x) / dist + 0.5f);

	m_value = m_range.clamp(value);
	update();

	ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

void Slider::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	const bool enabled = isEnable(true);
	const int32_t cy = rcInner.getCenter().y;
	const int32_t half = pixel(c_trackHeight) / 2;
	const int32_t radius = pixel(c_knobRadius);

	const int32_t value = m_range.clamp(m_value);
	const int32_t dist = rcInner.getSize().cx - pixel(c_margin) * 2;
	const int32_t knob = rcInner.left + pixel(c_margin) + int32_t(dist * float(value - m_range.min) / m_range.delta());

	// Track, as a rounded bar the full width.
	const Rect rcTrack(
		rcInner.left + pixel(c_margin), cy - half,
		rcInner.right - pixel(c_margin), cy + half
	);
	canvas.setBackground(ss->getColor(this, L"track-color"));
	canvas.fillRoundRect(rcTrack, half);

	// The part left of the knob shows how far along the value is.
	if (enabled && knob > rcTrack.left)
	{
		const Rect rcActive(rcTrack.left, rcTrack.top, knob, rcTrack.bottom);
		canvas.setBackground(ss->getColor(this, L"track-color-active"));
		canvas.fillRoundRect(rcActive, half);
	}

	const wchar_t* knobColor = L"knob-color";
	if (!enabled)
		knobColor = L"knob-color-disabled";
	else if (m_drag || m_hover)
		knobColor = L"knob-color-hover";

	canvas.setBackground(ss->getColor(this, knobColor));
	canvas.fillCircle(Point(knob, cy), (float)radius);

	canvas.setForeground(ss->getColor(this, L"knob-border-color"));
	canvas.drawCircle(Point(knob, cy), (float)radius);

	event->consume();
}

}
