/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Slider.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const Unit c_margin = 16_ut;
const Unit c_knobWidth = 6_ut;
const Unit c_knobMarginY = 3_ut;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Slider", Slider, Widget)

Slider::Slider()
:	m_range(0, 100)
,	m_value(0)
,	m_drag(false)
{
}

bool Slider::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent))
		return false;

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
	return Size(pixel(200_ut), pixel(20_ut));
}

Size Slider::getMaximumSize() const
{
	return Size(65535, pixel(20_ut));
}

void Slider::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	const int32_t value = m_range.clamp(m_value);
	const int32_t dist = sz.cx - pixel(c_margin) * 2;
	const int32_t knob = pixel(c_margin) + int32_t(dist * float(value - m_range.min) / m_range.delta());
	const int32_t knobL = knob - pixel(c_knobWidth) / 2;
	const int32_t knobR = knob + pixel(c_knobWidth) / 2;

	if (pt.x >= knobL && pt.x <= knobR)
	{
		setCapture();

		m_drag = true;
		update();
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

	const int32_t x = pt.x - pixel(c_margin);
	const int32_t dist = sz.cx - pixel(c_margin) * 2;
	const int32_t value = int32_t(m_range.delta() * float(x) / dist);

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
	canvas.drawLine(
		Point(rcInner.left + pixel(c_margin), rcInner.getCenter().y),
		Point(rcInner.right - pixel(c_margin), rcInner.getCenter().y)
	);

	const int32_t value = m_range.clamp(m_value);
	const int32_t dist = rcInner.getSize().cx - pixel(c_margin) * 2;
	const int32_t knob = pixel(c_margin) + int32_t(dist * float(value - m_range.min) / m_range.delta());
	const int32_t knobL = knob - pixel(c_knobWidth) / 2;
	const int32_t knobR = knob + pixel(c_knobWidth) / 2;

	const Rect rcKnob(
		Point(knobL, rcInner.top + pixel(c_knobMarginY)),
		Point(knobR, rcInner.bottom - pixel(c_knobMarginY))
	);

	canvas.setBackground(ss->getColor(this, L"knob-color"));
	canvas.fillRect(rcKnob);

	event->consume();
}

	}
}
