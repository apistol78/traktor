/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Button", Button, Widget)

Button::Button()
:	m_pushed(false)
,	m_hover(false)
{
}

bool Button::create(Widget* parent, const std::wstring& text, int style)
{
	if (!Widget::create(parent))
		return false;

	addEventHandler< MouseTrackEvent >(this, &Button::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &Button::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Button::eventButtonUp);
	addEventHandler< PaintEvent >(this, &Button::eventPaint);

	setText(text);
	return true;
}

void Button::setText(const std::wstring& text)
{
	Widget::setText(text);

	// Calculate prefered size from new text.
	const int32_t marginX = dpi96(16);
	const int32_t marginY = dpi96(4);

	FontMetric fm = getFontMetric();
	int32_t w = fm.getExtent(getText()).cx;
	int32_t h = fm.getHeight();

	m_preferedSize = Size(w + marginX * 2, h + marginY * 2);
}

Size Button::getPreferredSize(const Size& hint) const
{
	return m_preferedSize;
}

Size Button::getMaximumSize() const
{
	return m_preferedSize;
}

void Button::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void Button::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	m_pushed = true;
	update();
}

void Button::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!m_pushed || !isEnable())
		return;

	bool pushed = m_pushed;
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
	Canvas& canvas = event->getCanvas();
	Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	bool hover = isEnable() && m_hover;

	if (m_pushed)
		canvas.setBackground(ss->getColor(this, L"background-color-pushed"));
	else
		canvas.setBackground(ss->getColor(this, hover ? L"background-color-hover" : L"background-color"));

	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	if (m_pushed)
	{
		const int32_t offset = dpi96(1);
		rcInner = rcInner.offset(offset, offset);
	}

	canvas.setForeground(ss->getColor(this, isEnable() ? L"color" : L"color-disabled"));
	canvas.drawText(rcInner, getText(), AnCenter, AnCenter);

	event->consume();
}

	}
}
