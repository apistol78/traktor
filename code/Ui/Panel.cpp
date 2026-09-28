/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/StyleConstants.h"
#include "Ui/StyleSheet.h"
#include "Ui/Panel.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_panelMargin = 6_ut;

	}

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
	Application::getInstance()->removeEventHandler(m_focusEventHandler);
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
	const int32_t margin = pixel(c_panelMargin);
	Rect rc = Container::getInnerRect().inflate(-margin, -margin);
	rc.top += titleSize.cy + pixel(4_ut);
	return rc;
}

void Panel::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();
	const Rect rcInner = Widget::getInnerRect();
	const int32_t radius = pixel(c_surfaceRadius);

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRoundRect(rcInner, radius);

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRoundRect(rcInner, radius);

	const int32_t margin = pixel(c_panelMargin);
	const bool focus = containFocus();
	const std::wstring text = getText();
	const Size extent = canvas.getFontMetric().getExtent(text);
	const Rect rcTitle(rcInner.left, rcInner.top + pixel(4_ut), rcInner.right, rcInner.top + extent.cy + pixel(4_ut));
	canvas.setForeground(ss->getColor(this, focus ? L"caption-color-focus" : L"caption-color-no-focus"));
	canvas.drawText(
		rcTitle.inflate(-margin, 0),
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
