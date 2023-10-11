/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
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
#include "Ui/ToolBar/ToolBarDropDown.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarDropDown", ToolBarDropDown, ToolBarItem)

ToolBarDropDown::ToolBarDropDown(const Command& command, Unit width, const std::wstring& toolTip)
:	m_command(command)
,	m_width(width)
,	m_toolTip(toolTip)
,	m_selected(-1)
,	m_hover(false)
,	m_dropPosition(0)
,	m_menuWidth(0)
{
}

int32_t ToolBarDropDown::add(const std::wstring& item, Object* data)
{
	m_items.push_back({ item, data });
	return int32_t(m_items.size() - 1);
}

bool ToolBarDropDown::remove(int32_t index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	auto it = m_items.begin() + index;
	m_items.erase(it);

	if (index >= m_selected)
		m_selected = -1;

	return true;
}

void ToolBarDropDown::removeAll()
{
	m_items.resize(0);
	m_selected = -1;
}

int32_t ToolBarDropDown::count() const
{
	return (int32_t)m_items.size();
}

std::wstring ToolBarDropDown::getItem(int32_t index) const
{
	if (index < 0 || index >= (int32_t)m_items.size())
		return L"";

	return m_items[index].text;
}

Object* ToolBarDropDown::getData(int32_t index) const
{
	if (index < 0 || index >= (int32_t)m_items.size())
		return nullptr;

	return m_items[index].data;
}

void ToolBarDropDown::select(int32_t index)
{
	m_selected = index;
}

int32_t ToolBarDropDown::getSelected() const
{
	return m_selected;
}

std::wstring ToolBarDropDown::getSelectedItem() const
{
	return getItem(m_selected);
}

Object* ToolBarDropDown::getSelectedData() const
{
	return getData(m_selected);
}

bool ToolBarDropDown::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarDropDown::getSize(const ToolBar* toolBar) const
{
	const Size imageSize = toolBar->getImageSize();
	return Size(toolBar->pixel(m_width), imageSize.cy + toolBar->pixel(4_ut));
}

void ToolBarDropDown::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images)
{
	const StyleSheet* ss = toolBar->getStyleSheet();
	const Size size = getSize(toolBar);
	const bool enabled = isEnable() && toolBar->isEnable(true);
	const int32_t sep = toolBar->pixel(14_ut);

	const Rect rcText(
		at.x + 4,
		at.y + 2,
		at.x + size.cx - sep - 2,
		at.y + size.cy - 2
	);
	const Rect rcButton(
		at.x + size.cx - sep,
		at.y + 1,
		at.x + size.cx - 1,
		at.y + size.cy - 1
	);

	canvas.setBackground(ss->getColor(toolBar, m_hover ? L"item-background-color-dropdown-hover" : L"item-background-color-dropdown"));
	canvas.fillRect(Rect(at, size));

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
		Point(center.x - toolBar->pixel(3_ut), center.y - toolBar->pixel(1_ut)),
		Point(center.x + toolBar->pixel(2_ut), center.y - toolBar->pixel(1_ut)),
		Point(center.x - toolBar->pixel(1_ut), center.y + toolBar->pixel(2_ut))
	};

	canvas.setBackground(ss->getColor(toolBar, L"item-color-dropdown-arrow"));
	canvas.fillPolygon(pnts, 3);

	canvas.setForeground(ss->getColor(toolBar, enabled ? L"color" : L"color-disabled"));
	canvas.drawText(rcText, getSelectedItem(), AnLeft, AnCenter);

	m_dropPosition = rcButton.left;
	m_menuPosition = Point(at.x, at.y + size.cy);
	m_menuWidth = size.cx;
}

bool ToolBarDropDown::mouseEnter(ToolBar* toolBar)
{
	m_hover = true;
	return true;
}

void ToolBarDropDown::mouseLeave(ToolBar* toolBar)
{
	m_hover = false;
}

void ToolBarDropDown::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
	if (m_items.empty())
		return;

	Menu menu;
	for (uint32_t i = 0; i < uint32_t(m_items.size()); ++i)
		menu.add(new MenuItem(Command(i), m_items[i].text));

	const MenuItem* selectedItem = menu.showModal(toolBar, m_menuPosition, m_menuWidth);
	if (selectedItem != nullptr)
	{
		m_selected = selectedItem->getCommand().getId();

		ToolBarButtonClickEvent clickEvent(toolBar, this, m_command);
		toolBar->raiseEvent(&clickEvent);
	}
}

void ToolBarDropDown::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

}
