/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
{
}

bool Button::create(Widget* parent, const std::wstring& text, int style)
{
	if (!Widget::create(parent))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &Button::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Button::eventButtonUp);
	addEventHandler< PaintEvent >(this, &Button::eventPaint);

	setText(text);
	return true;
}

void Button::setState(bool state)
{
}

bool Button::getState() const
{
	return false;
}

Size Button::getPreferedSize() const
{
	const int32_t marginX = dpi96(16);
	const int32_t marginY = dpi96(4);
	return getTextExtent(getText()) + Size(marginX * 2, marginY * 2);
}

Size Button::getMaximumSize() const
{
	return getPreferedSize();
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

	if (m_pushed)
	{
		ButtonClickEvent clickEvent(this);
		raiseEvent(&clickEvent);
	}

	m_pushed = false;
	update();
}

void Button::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();
	
	Rect rcInner = getInnerRect();
	
	canvas.setBackground(ss->getColor(this, m_pushed ? L"background-color-pushed" : L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	if (m_pushed)
	{
		const int32_t offset = dpi96(1);
		rcInner = rcInner.offset(offset, offset);
	}

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(rcInner, getText(), AnCenter, AnCenter);

	event->consume();
}

	}
}
