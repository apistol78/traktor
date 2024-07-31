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
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropMenu.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarDropMenu", ToolBarDropMenu, ToolBarItem)

ToolBarDropMenu::ToolBarDropMenu(Unit width, const std::wstring& text, bool arrow, const std::wstring& toolTip)
:	m_width(width)
,	m_text(text)
,	m_arrow(arrow)
,	m_toolTip(toolTip)
,	m_hover(false)
,	m_dropPosition(0)
{
}

int32_t ToolBarDropMenu::add(MenuItem* item)
{
	const int32_t index = (int32_t)(m_items.size() - 1);
	m_items.push_back(item);
	return index;
}

bool ToolBarDropMenu::remove(int32_t index)
{
	if (index >= (int32_t)m_items.size())
		return false;

	auto it = m_items.begin() + index;
	m_items.erase(it);

	return true;
}

void ToolBarDropMenu::removeAll()
{
	m_items.resize(0);
}

int32_t ToolBarDropMenu::count() const
{
	return (int32_t)m_items.size();
}

MenuItem* ToolBarDropMenu::get(int32_t index) const
{
	return (index >= 0 && index < (int32_t)m_items.size()) ? m_items[index] : nullptr;
}

bool ToolBarDropMenu::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarDropMenu::getSize(const ToolBar* toolBar) const
{
	const Size imageSize = toolBar->getImageSize();
	const Size sz = toolBar->getFontMetric().getExtent(m_text);
	if (m_width > 0_ut)
		return Size(toolBar->pixel(m_width), std::max< int32_t >(sz.cy, imageSize.cy) + toolBar->pixel(4_ut));
	else
		return Size(sz.cx + toolBar->pixel(32_ut), std::max< int32_t >(sz.cy, imageSize.cy) + toolBar->pixel(4_ut));
}

void ToolBarDropMenu::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images)
{
	const StyleSheet* ss = toolBar->getStyleSheet();
	const Size size = getSize(toolBar);

	const int32_t sep = toolBar->pixel(14_ut);

	Rect rcText;
	if (m_arrow)
		rcText = Rect(
			at.x + toolBar->pixel(4_ut),
			at.y + 2,
			at.x + size.cx - sep - 2,
			at.y + size.cy - 2
		);
	else
		rcText = Rect(
			at.x + 2,
			at.y + 2,
			at.x + size.cx - 2,
			at.y + size.cy - 2
		);

	canvas.setBackground(ss->getColor(toolBar, m_hover ? L"item-background-color-dropdown-hover" : L"item-background-color-dropdown"));
	canvas.fillRect(Rect(at, size));

	if (m_arrow)
	{
		const Rect rcButton(
			at.x + size.cx - sep,
			at.y + 1,
			at.x + size.cx - 1,
			at.y + size.cy - 1
		);

		canvas.setBackground(ss->getColor(toolBar, L"item-background-color-dropdown-button"));
		canvas.fillRect(rcButton);

		if (m_hover)
		{
			canvas.setForeground(ss->getColor(toolBar, L"item-color-dropdown-hover"));
			canvas.drawRect(Rect(at, size));
			canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom);
		}

		const Point center = rcButton.getCenter();
		const Point pnts[] =
		{
			ui::Point(center.x - toolBar->pixel(3_ut), center.y - toolBar->pixel(1_ut)),
			ui::Point(center.x + toolBar->pixel(2_ut), center.y - toolBar->pixel(1_ut)),
			ui::Point(center.x - toolBar->pixel(1_ut), center.y + toolBar->pixel(2_ut))
		};

		canvas.setBackground(ss->getColor(toolBar, L"item-color-dropdown-arrow"));
		canvas.fillPolygon(pnts, 3);

		m_dropPosition = rcButton.left;
	}
	else
		m_dropPosition = 0;

	canvas.setForeground(ss->getColor(toolBar, L"color"));
	canvas.drawText(rcText, m_text, m_arrow ? AnLeft : AnCenter, AnCenter);

	m_menuPosition = Point(at.x, at.y + size.cy);
}

bool ToolBarDropMenu::mouseEnter(ToolBar* toolBar)
{
	m_hover = true;
	return true;
}

void ToolBarDropMenu::mouseLeave(ToolBar* toolBar)
{
	m_hover = false;
}

void ToolBarDropMenu::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
	if (m_items.empty())
		return;

	Menu menu;

	for (size_t i = 0; i < m_items.size(); ++i)
		menu.add(m_items[i]);

	const MenuItem* item = menu.showModal(toolBar, m_menuPosition);
	if (item != nullptr)
	{
		ToolBarButtonClickEvent clickEvent(toolBar, this, item->getCommand());
		toolBar->raiseEvent(&clickEvent);
	}

	toolBar->update();
}

void ToolBarDropMenu::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

}
