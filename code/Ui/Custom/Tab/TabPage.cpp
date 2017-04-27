/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Tab/TabPage.h"
#include "Ui/Custom/Tab/Tab.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TabPage", TabPage, Container)

bool TabPage::create(Tab* tab, const std::wstring& text, Layout* layout)
{
	if (!Container::create(tab, WsNone, layout))
		return false;

	addEventHandler< PaintEvent >(this, &TabPage::eventPaint);

	m_tab = tab;

	setText(text);

	return true;
}

void TabPage::setActive()
{
	m_tab->setActivePage(this);
}

bool TabPage::isActive() const
{
	return bool(m_tab->getActivePage() == this);
}

Ref< Tab > TabPage::getTab()
{
	return m_tab;
}

void TabPage::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rcInner = Widget::getInnerRect();

	canvas.setBackground(Color4ub(224, 224, 224));
	canvas.setForeground(Color4ub(196, 194, 195));
	canvas.fillGradientRect(rcInner, true);

	event->consume();
}

		}
	}
}
