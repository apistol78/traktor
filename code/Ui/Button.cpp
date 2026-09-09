/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Canvas.h"
#include "Ui/StyleConstants.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_marginX = 16_ut;
const Unit c_marginY = 6_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Button", Button, Widget)

bool Button::create(Widget* parent, const std::wstring& text, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	m_defaultButton = ((style & WsDefaultButton) != 0);

	addEventHandler< MouseTrackEvent >(this, &Button::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &Button::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Button::eventButtonUp);
	addEventHandler< PaintEvent >(this, &Button::eventPaint);

	setText(text);
	return true;
}

Size Button::getPreferredSize(const Size& hint) const
{
	// Calculate preferred size from new text.
	const FontMetric fm = getFontMetric();
	const Size xt = fm.getExtent(getText());

	return Size(
		xt.cx + pixel(c_marginX) * 2,
		xt.cy + pixel(c_marginY) * 2
	);
}

Size Button::getMaximumSize() const
{
	return getPreferredSize(Size(0, 0));
}

void Button::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void Button::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable(true))
		return;

	m_pushed = true;
	update();
}

void Button::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!m_pushed || !isEnable(true))
		return;

	const bool pushed = m_pushed;
	m_pushed = false;
	update();

	if (pushed)
	{
		ButtonClickEvent clickEvent(this);
		raiseEvent(&clickEvent);
	}
}

void Button::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = getStyleSheet();
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const int32_t radius = pixel(c_controlRadius);

	const bool enabled = isEnable(true);
	const bool hover = enabled && m_hover;

	// Cover the whole client area first; the rounded button leaves the four
	// corners of the rectangle uncovered.
	canvas.setBackground(ss->getColor(getParent(), L"background-color"));
	canvas.fillRect(rcInner);

	// The default button of a dialog is filled with the theme accent.
	const wchar_t* background;
	if (!enabled)
		background = L"background-color-disabled";
	else if (m_defaultButton)
		background = m_pushed ? L"background-color-default-pushed" : (hover ? L"background-color-default-hover" : L"background-color-default");
	else
		background = m_pushed ? L"background-color-pushed" : (hover ? L"background-color-hover" : L"background-color");

	canvas.setBackground(ss->getColor(this, background));
	canvas.fillRoundRect(rcInner, radius);

	// An accent fill needs no outline; it already reads as a solid shape.
	if (!(enabled && m_defaultButton))
	{
		canvas.setForeground(ss->getColor(this, enabled ? L"border-color" : L"border-color-disabled"));
		canvas.drawRoundRect(rcInner, radius);
	}

	const wchar_t* color;
	if (!enabled)
		color = L"color-disabled";
	else
		color = m_defaultButton ? L"color-default" : L"color";

	canvas.setForeground(ss->getColor(this, color));
	canvas.drawText(rcInner, getText(), AnCenter, AnCenter);

	event->consume();
}

}
