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
	namespace
	{

const Unit c_fieldPad = 8_ut;
const Unit c_chevronArea = 18_ut;
const Unit c_chevronHalf = 3_ut;

void paintChevron(ToolBar* toolBar, Canvas& canvas, const Point& center)
{
	const int32_t h = toolBar->pixel(c_chevronHalf);
	canvas.setPenThickness(toolBar->pixel(2_ut));
	canvas.drawLine(center.x - h, center.y - h / 2, center.x, center.y + h - h / 2);
	canvas.drawLine(center.x, center.y + h - h / 2, center.x + h, center.y - h / 2);
	canvas.setPenThickness(1);
}

	}

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
	return Size(toolBar->pixel(m_width), imageSize.cy + toolBar->pixel(12_ut));
}

void ToolBarDropDown::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images)
{
	const StyleSheet* ss = toolBar->getStyleSheet();
	const Size size = getSize(toolBar);
	const bool enabled = isEnable() && toolBar->isEnable(true);
	const int32_t radius = toolBar->getItemRadius();
	const int32_t chevron = toolBar->pixel(c_chevronArea);
	const int32_t pad = toolBar->pixel(c_fieldPad);

	const Rect rcField(at, size);
	const Rect rcText(
		at.x + pad,
		at.y,
		at.x + size.cx - chevron,
		at.y + size.cy
	);

	canvas.setBackground(ss->getColor(toolBar, m_hover ? L"item-background-color-dropdown-hover" : L"item-background-color-dropdown"));
	canvas.fillRoundRect(rcField, radius);

	canvas.setForeground(ss->getColor(toolBar, m_hover ? L"item-color-dropdown-hover" : L"border-color"));
	canvas.drawRoundRect(rcField, radius);

	canvas.setForeground(ss->getColor(toolBar, L"item-color-dropdown-arrow"));
	paintChevron(toolBar, canvas, Point(at.x + size.cx - chevron / 2, at.y + size.cy / 2));

	canvas.setForeground(ss->getColor(toolBar, enabled ? L"color" : L"color-disabled"));
	canvas.drawText(rcText, getSelectedItem(), AnLeft, AnCenter);

	m_dropPosition = at.x + size.cx - chevron;
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
