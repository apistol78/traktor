/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarMenu.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarMenu", ToolBarMenu, ToolBarItem)

ToolBarMenu::ToolBarMenu(const std::wstring& text, const std::wstring& toolTip)
:	m_text(text)
,	m_toolTip(toolTip)
,	m_hover(false)
{
}

int32_t ToolBarMenu::add(MenuItem* item)
{
	int32_t index = int32_t(m_items.size() - 1);
	m_items.push_back(item);
	return index;
}

bool ToolBarMenu::remove(int32_t index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	RefArray< MenuItem >::iterator i = m_items.begin() + index;
	m_items.erase(i);

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
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : 0;
}

bool ToolBarMenu::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_toolTip;
	return !outToolTip.empty();
}

Size ToolBarMenu::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	Size sz = toolBar->getTextExtent(m_text);
	return Size(sz.cx + scaleBySystemDPI(16), imageHeight + scaleBySystemDPI(4));
}

void ToolBarMenu::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Size size = getSize(toolBar, imageWidth, imageHeight);

	Rect rcText(
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

bool ToolBarMenu::mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent)
{
	m_hover = true;
	return true;
}

void ToolBarMenu::mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent)
{
	m_hover = false;
}

void ToolBarMenu::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
	if (m_items.empty())
		return;

	if (!m_menu)
	{
		m_menu = new PopupMenu();
		if (m_menu->create())
		{
			for (size_t i = 0; i < m_items.size(); ++i)
				m_menu->add(m_items[i]);
		
			Ref< MenuItem > item = m_menu->show(toolBar, m_menuPosition);
			if (item)
			{
				ToolBarButtonClickEvent clickEvent(toolBar, this, item->getCommand());
				toolBar->raiseEvent(&clickEvent);
			}

			safeDestroy(m_menu);
			toolBar->update();
		}
		else
			m_menu = 0;
	}
	else
	{
		safeDestroy(m_menu);
	}
}

void ToolBarMenu::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

		}
	}
}
