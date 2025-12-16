/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Slider.h"

#include "Ui/Application.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
namespace
{

const Unit c_margin = 8_ut;
const Unit c_knobWidth = 6_ut;
const Unit c_knobMarginY = 3_ut;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Slider", Slider, Widget)

bool Slider::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &Slider::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Slider::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &Slider::eventMouseMove);
	addEventHandler< PaintEvent >(this, &Slider::eventPaint);

	m_vertical = ((style & WsVertical) == WsVertical);
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
	return !m_vertical ? pixel(UnitSize(100_ut, 20_ut)) : pixel(UnitSize(20_ut, 100_ut));
}

Size Slider::getMaximumSize() const
{
	return !m_vertical ? Size(65535, pixel(20_ut)) : Size(pixel(20_ut), 65535);
}

void Slider::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable(true))
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	const int32_t szx = !m_vertical ? sz.cx : sz.cy;
	const int32_t ptx = !m_vertical ? pt.x : pt.y;

	const int32_t value = m_range.clamp(m_value);
	float frac = float(value - m_range.min) / m_range.delta();
	if (m_vertical)
		frac = 1.0f - frac;

	const int32_t dist = szx - pixel(c_margin) * 2;
	const int32_t knob = pixel(c_margin) + int32_t(dist * frac);
	const int32_t knobL = knob - pixel(c_knobWidth) / 2;
	const int32_t knobR = knob + pixel(c_knobWidth) / 2;

	if (ptx >= knobL && ptx <= knobR)
	{
		setCapture();

		m_drag = true;
		update();
	}
	else if (ptx < knobL)
	{
		if (!m_vertical)
			m_value = m_range.clamp(m_value - m_range.delta() / 10);
		else
			m_value = m_range.clamp(m_value + m_range.delta() / 10);

		update();

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
	else if (ptx > knobR)
	{
		if (!m_vertical)
			m_value = m_range.clamp(m_value + m_range.delta() / 10);
		else
			m_value = m_range.clamp(m_value - m_range.delta() / 10);

		update();

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
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

	const int32_t szx = !m_vertical ? sz.cx : sz.cy;
	const int32_t ptx = !m_vertical ? pt.x : pt.y;

	const int32_t x = ptx - pixel(c_margin);
	const int32_t dist = szx - pixel(c_margin) * 2;

	float frac = float(x) / dist;
	if (m_vertical)
		frac = 1.0f - frac;

	const int32_t value = int32_t(m_range.delta() * frac + 0.5f);

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

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"color"));
	if (!m_vertical)
		canvas.drawLine(
			Point(rcInner.left + pixel(c_margin), rcInner.getCenter().y),
			Point(rcInner.right - pixel(c_margin), rcInner.getCenter().y));
	else
		canvas.drawLine(
			Point(rcInner.getCenter().x, rcInner.top + pixel(c_margin)),
			Point(rcInner.getCenter().x, rcInner.bottom - pixel(c_margin)));

	const int32_t rcx = !m_vertical ? rcInner.getSize().cx : rcInner.getSize().cy;

	const int32_t value = m_range.clamp(m_value);
	float frac = float(value - m_range.min) / m_range.delta();
	if (m_vertical)
		frac = 1.0f - frac;

	const int32_t dist = rcx - pixel(c_margin) * 2;
	const int32_t knob = pixel(c_margin) + int32_t(dist * frac);

	const int32_t knobL = knob - pixel(c_knobWidth) / 2;
	const int32_t knobR = knob + pixel(c_knobWidth) / 2;

	const Rect rcKnob = !m_vertical ? Rect(
										  Point(knobL, rcInner.top + pixel(c_knobMarginY)),
										  Point(knobR, rcInner.bottom - pixel(c_knobMarginY)))
									: Rect(
										  Point(rcInner.left + pixel(c_knobMarginY), knobL),
										  Point(rcInner.right - pixel(c_knobMarginY), knobR));

	canvas.setBackground(ss->getColor(this, L"knob-color"));
	canvas.fillRect(rcKnob);

	event->consume();
}

}
