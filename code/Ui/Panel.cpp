/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Panel.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Panel", Panel, Container)

bool Panel::create(Widget* parent, const std::wstring& text, Layout* layout)
{
	if (!Container::create(parent, WsNone, layout))
		return false;

	setText(text);

	addEventHandler< PaintEvent >(this, &Panel::eventPaint);

	m_focusEventHandler = Application::getInstance()->addEventHandler< FocusEvent >(this, &Panel::eventFocus);

	return true;
}

void Panel::destroy()
{
	Application::getInstance()->removeEventHandler< FocusEvent >(m_focusEventHandler);
	Widget::destroy();
}

Size Panel::getMinimumSize() const
{
	const Size titleSize = getFontMetric().getExtent(getText());
	Size sz = Container::getMinimumSize();
	sz.cx += pixel(2_ut);
	sz.cy += pixel(8_ut) + titleSize.cy;
	return sz;
}

Size Panel::getPreferredSize(const Size& hint) const
{
	const Size titleSize = getFontMetric().getExtent(getText());
	Size sz = Container::getPreferredSize(hint);
	sz.cx += pixel(2_ut);
	sz.cy += pixel(8_ut) + titleSize.cy;
	return sz;
}

Rect Panel::getInnerRect() const
{
	const Size titleSize = getFontMetric().getExtent(getText());
	Rect rc = Container::getInnerRect();
	rc.left += pixel(1_ut);
	rc.top += titleSize.cy + pixel(8_ut);
	rc.right -= pixel(1_ut);
	rc.bottom -= pixel(1_ut);
	return rc;
}

void Panel::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = Widget::getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(rcInner);

	const bool focus = containFocus();

	const std::wstring text = getText();
	const Size extent = canvas.getFontMetric().getExtent(text);

	const Rect rcTitle(rcInner.left, rcInner.top, rcInner.right, rcInner.top + extent.cy + pixel(8_ut));

	canvas.setBackground(ss->getColor(this, focus ? L"caption-background-color-focus" : L"caption-background-color-no-focus"));
	canvas.fillRect(rcTitle);

	canvas.setForeground(ss->getColor(this, focus ? L"caption-color-focus" : L"caption-color-no-focus"));
	canvas.drawText(
		rcTitle.inflate(pixel(-4_ut), 0),
		text,
		AnLeft,
		AnCenter
	);

	event->consume();
}

void Panel::eventFocus(FocusEvent* event)
{
	update();
}

}
