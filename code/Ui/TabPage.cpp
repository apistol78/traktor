/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleConstants.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_pageMargin = 6_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabPage", TabPage, Container)

bool TabPage::create(Tab* tab, const std::wstring& text, const std::wstring& toolTip, int32_t imageIndex, Layout* layout)
{
	m_tab = tab;
	m_imageIndex = imageIndex;
	m_toolTip = toolTip;

	if (!Container::create(tab, WsNone, layout))
		return false;

	addEventHandler< PaintEvent >(this, &TabPage::eventPaint);

	setText(text);
	return true;
}

bool TabPage::create(Tab* tab, const std::wstring& text, const std::wstring& toolTip, Layout* layout)
{
	return create(tab, text, toolTip, 0, layout);
}

bool TabPage::create(Tab* tab, const std::wstring& text, int32_t imageIndex, Layout* layout)
{
	return create(tab, text, L"", imageIndex, layout);
}

bool TabPage::create(Tab* tab, const std::wstring& text, Layout* layout)
{
	return create(tab, text, L"", 0, layout);
}

void TabPage::setActive()
{
	m_tab->setActivePage(this);
}

bool TabPage::isActive() const
{
	return bool(m_tab->getActivePage() == this);
}

Tab* TabPage::getTab() const
{
	return m_tab;
}

int32_t TabPage::getImageIndex() const
{
	return m_imageIndex;
}

bool TabPage::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !m_toolTip.empty();
}

Rect TabPage::getInnerRect() const
{
	const int32_t margin = pixel(c_pageMargin);
	return Widget::getInnerRect().inflate(-margin, -margin);
}

void TabPage::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();
	const Rect rcInner = Widget::getInnerRect();
	const int32_t radius = pixel(c_surfaceRadius);

	// Cover the whole client area first with the surrounding colour; the
	// rounded page leaves the four corners of the rectangle uncovered.
	canvas.setBackground(ss->getColor(getParent(), L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRoundRect(rcInner, radius);

	canvas.setForeground(ss->getColor(this, L"page-border-color"));
	canvas.drawRoundRect(rcInner, radius);
}

}
