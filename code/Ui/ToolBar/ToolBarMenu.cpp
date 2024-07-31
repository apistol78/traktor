/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarMenu.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarMenu", ToolBarMenu, ToolBarItem)

ToolBarMenu::ToolBarMenu(const std::wstring& text, const std::wstring& toolTip)
:	m_text(text)
,	m_toolTip(toolTip)
{
}

int32_t ToolBarMenu::add(MenuItem* item)
{
	const int32_t index = (int32_t)(m_items.size() - 1);
	m_items.push_back(item);
	return index;
}

bool ToolBarMenu::remove(int32_t index)
{
	if (index >= (int32_t)m_items.size())
		return false;

	auto it = m_items.begin() + index;
	m_items.erase(it);

	return true;
}

void ToolBarMenu::removeAll()
{
	m_items.resize(0);
}

int32_t ToolBarMenu::count() const
{
	return int32_t(m_items.size());
}

MenuItem* ToolBarMenu::get(int32_t index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : nullptr;
}

bool ToolBarMenu::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarMenu::getSize(const ToolBar* toolBar) const
{
	const Size imageSize = toolBar->getImageSize();
	const Size sz = toolBar->getFontMetric().getExtent(m_text);
	return Size(sz.cx + toolBar->pixel(24_ut), std::max< int32_t >(sz.cy, imageSize.cy) + toolBar->pixel(8_ut));
}

void ToolBarMenu::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images)
{
	const StyleSheet* ss = toolBar->getStyleSheet();
	const Size size = getSize(toolBar);

	const Rect rcText(
		at.x + 2,
		at.y + 2,
		at.x + size.cx - 2,
		at.y + size.cy - 2
	);

	if (m_hover)
	{
		canvas.setBackground(ss->getColor(toolBar, L"item-background-color-hover"));
		canvas.fillRect(Rect(at, size));
	}

	canvas.setForeground(ss->getColor(toolBar, L"color"));
	canvas.drawText(rcText, m_text, AnCenter, AnCenter);

	m_menuPosition = Point(at.x, at.y + size.cy);
}

bool ToolBarMenu::mouseEnter(ToolBar* toolBar)
{
	m_hover = true;
	return true;
}

void ToolBarMenu::mouseLeave(ToolBar* toolBar)
{
	m_hover = false;
}

void ToolBarMenu::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
	// Do not create another menu if already is active.
	if (m_active)
		return;

	if (m_items.empty())
		return;

	Ref< Menu > menu = new Menu();
	for (size_t i = 0; i < m_items.size(); ++i)
		menu->add(m_items[i]);

	m_active = true;

	const MenuItem* item = menu->showModal(toolBar, m_menuPosition);
	if (item)
	{
		ToolBarButtonClickEvent clickEvent(toolBar, this, item->getCommand(), item);
		toolBar->raiseEvent(&clickEvent);
	}

	m_active = false;

	toolBar->update();
}

void ToolBarMenu::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

}
