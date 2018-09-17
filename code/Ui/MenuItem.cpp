/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"
#include "Ui/Events/MenuClickEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_itemMarginX = 12;
const int32_t c_itemMarginY = 8;
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuItem", MenuItem, Object)

MenuItem::MenuItem(const Command& command, const std::wstring& text, bool checkBox, Bitmap* image)
:	m_command(command)
,	m_text(text)
,	m_checkBox(checkBox)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
,	m_hover(false)
{
}

MenuItem::MenuItem(const std::wstring& text, bool checkBox, Bitmap* image)
:	m_text(text)
,	m_checkBox(checkBox)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
,	m_hover(false)
{
}

MenuItem::MenuItem(const Command& command, const std::wstring& text, Bitmap* image)
:	m_command(command)
,	m_text(text)
,	m_checkBox(false)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
,	m_hover(false)
{
}

MenuItem::MenuItem(const std::wstring& text, Bitmap* image)
:	m_text(text)
,	m_checkBox(false)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
,	m_hover(false)
{
}

MenuItem::MenuItem(const Command& command, const std::wstring& text)
:	m_command(command)
,	m_text(text)
,	m_checkBox(false)
,	m_enable(true)
,	m_checked(false)
,	m_hover(false)
{
}

MenuItem::MenuItem(const std::wstring& text)
:	m_text(text)
,	m_checkBox(false)
,	m_enable(true)
,	m_checked(false)
,	m_hover(false)
{
}

void MenuItem::setCommand(const Command& command)
{
	m_command = command;
}

const Command& MenuItem::getCommand() const
{
	return m_command;
}

void MenuItem::setText(const std::wstring& text)
{
	m_text = text;
}

void MenuItem::setCheckBox(bool checkBox)
{
	m_checkBox = checkBox;
}

bool MenuItem::getCheckBox() const
{
	return m_checkBox;
}

const std::wstring& MenuItem::getText() const
{
	return m_text;
}

void MenuItem::setImage(Bitmap* image)
{
	m_image = image;
}

Ref< Bitmap > MenuItem::getImage() const
{
	return m_image;
}

void MenuItem::setEnable(bool enable)
{
	m_enable = enable;
}

bool MenuItem::isEnable() const
{
	return m_enable;
}

void MenuItem::setChecked(bool checked)
{
	m_checked = checked;
}

bool MenuItem::isChecked() const
{
	return m_checked;
}

void MenuItem::remove(MenuItem* item)
{
	RefArray< MenuItem >::iterator i = std::find(m_items.begin(), m_items.end(), item);
	m_items.erase(i);
}

void MenuItem::removeAll()
{
	m_items.resize(0);
}

void MenuItem::add(MenuItem* item)
{
	m_items.push_back(item);
}

int MenuItem::count() const
{
	return int(m_items.size());
}

Ref< MenuItem > MenuItem::get(int index)
{
	if (index < 0 || index >= int(m_items.size()))
		return 0;

	return m_items[index];
}

Size MenuItem::getSize(const Widget* shell) const
{
	if (m_text != L"-")
	{
		int32_t cw = shell->getFontMetric().getExtent(m_text).cx;
		int32_t ch = shell->getFontMetric().getHeight();
		return Size(cw + dpi96(c_itemMarginX) * 2, ch + dpi96(c_itemMarginY) * 2);
	}
	else
		return Size(0, 1 + dpi96(c_itemMarginY) * 2);
}

void MenuItem::paint(const Widget* shell, Canvas& canvas, const Rect& rc) const
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	const Size sz = getSize(shell);

	Rect rcLabel = rc.inflate(-dpi96(c_itemMarginX), 0);

	canvas.setBackground(ss->getColor(this, m_hover ? L"background-color-hover" : L"background-color"));
	canvas.fillRect(rc);

	canvas.setForeground(ss->getColor(this, L"color"));

	if (m_text != L"-")
		canvas.drawText(rcLabel, m_text, AnLeft, AnCenter);
	else
		canvas.drawLine(
			Point(rcLabel.left, rcLabel.getCenter().y),
			Point(rcLabel.right, rcLabel.getCenter().y)
		);
}

bool MenuItem::mouseEnter(Widget* shell, MouseMoveEvent* mouseEvent)
{
	if (m_text != L"-")
		m_hover = true;
	return true;
}

void MenuItem::mouseLeave(Widget* shell, MouseMoveEvent* mouseEvent)
{
	m_hover = false;
}

void MenuItem::buttonDown(Widget* shell, MouseButtonDownEvent* mouseEvent)
{
}

void MenuItem::buttonUp(Widget* shell, MouseButtonUpEvent* mouseEvent)
{
	MenuClickEvent clickEvent(shell, this, m_command);
	shell->raiseEvent(&clickEvent);
}

	}
}
