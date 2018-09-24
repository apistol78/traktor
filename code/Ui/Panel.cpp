/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Panel.h"

namespace traktor
{
	namespace ui
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
	Size titleSize = getFontMetric().getExtent(getText());
	Size sz = Container::getMinimumSize();
	sz.cx += 2;
	sz.cy += 1 + titleSize.cy;
	return sz;
}

Size Panel::getPreferedSize() const
{
	Size titleSize = getFontMetric().getExtent(getText());
	Size sz = Container::getPreferedSize();
	sz.cx += 2;
	sz.cy += 3 + titleSize.cy;
	return sz;
}

Rect Panel::getInnerRect() const
{
	Size titleSize = getFontMetric().getExtent(getText());
	Rect rc = Container::getInnerRect();
	rc.left += 1;
	rc.top += titleSize.cy + 3;
	rc.right -= 1;
	rc.bottom -= 1;
	return rc;
}

void Panel::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();

	Rect rcInner = Widget::getInnerRect();
	canvas.fillRect(rcInner);

	bool focus = containFocus();

	std::wstring text = getText();
	Size extent = canvas.getFontMetric().getExtent(text);

	Rect rcTitle(rcInner.left, rcInner.top, rcInner.right, rcInner.top + extent.cy + 4);

	canvas.setBackground(ss->getColor(this, focus ? L"caption-background-color-focus" : L"caption-background-color-no-focus"));
	canvas.fillRect(rcTitle);

	canvas.setForeground(ss->getColor(this, focus ? L"caption-color-focus" : L"caption-color-no-focus"));
	canvas.drawText(
		rcTitle.inflate(-4, 0),
		text,
		AnLeft,
		AnCenter
	);

	Point pntBorder[5] =
	{
		Point(rcInner.left, rcInner.top),
		Point(rcInner.right - 1, rcInner.top),
		Point(rcInner.right - 1, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.top)
	};
	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLines(pntBorder, 5);

	event->consume();
}

void Panel::eventFocus(FocusEvent* event)
{
	update();
}

	}
}
