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

const int32_t c_margin = 16;
const int32_t c_knobWidth = 6;
const int32_t c_knobMarginY = 3;

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
	return Size(dpi96(200), dpi96(20));
}

Size Slider::getMaximumSize() const
{
	return Size(65535, dpi96(20));
}

void Slider::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	auto sz = getInnerRect().getSize();
	const auto& pt = event->getPosition();

	int32_t value = m_range.clamp(m_value);
	int32_t dist = sz.cx - dpi96(c_margin) * 2;
	int32_t knob = dpi96(c_margin) + int32_t(dist * float(value - m_range.min) / m_range.delta());
	int32_t knobL = knob - dpi96(c_knobWidth) / 2;
	int32_t knobR = knob + dpi96(c_knobWidth) / 2;

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

	int32_t x = pt.x - dpi96(c_margin);
	int32_t dist = sz.cx - dpi96(c_margin) * 2;
	int32_t value = int32_t(m_range.delta() * float(x) / dist);

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
		Point(rcInner.left + dpi96(c_margin), rcInner.getCenter().y),
		Point(rcInner.right - dpi96(c_margin), rcInner.getCenter().y)
	);

	int32_t value = m_range.clamp(m_value);
	int32_t dist = rcInner.getSize().cx - dpi96(c_margin) * 2;
	int32_t knob = dpi96(c_margin) + int32_t(dist * float(value - m_range.min) / m_range.delta());
	int32_t knobL = knob - dpi96(c_knobWidth) / 2;
	int32_t knobR = knob + dpi96(c_knobWidth) / 2;

	Rect rcKnob(
		Point(knobL, rcInner.top + dpi96(c_knobMarginY)),
		Point(knobR, rcInner.bottom - dpi96(c_knobMarginY))
	);

	canvas.setBackground(ss->getColor(this, L"knob-color"));
	canvas.fillRect(rcKnob);

	event->consume();
}

	}
}
