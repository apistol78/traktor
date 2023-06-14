/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"
#include "Ui/Events/MenuClickEvent.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_itemMarginX = 12_ut;
const Unit c_itemMarginY = 6_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuItem", MenuItem, Object)

MenuItem::MenuItem(const Command& command, const std::wstring& text, bool checkBox, IBitmap* image)
:	m_command(command)
,	m_text(text)
,	m_checkBox(checkBox)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
{
	setCheckBox(checkBox);
}

MenuItem::MenuItem(const std::wstring& text, bool checkBox, IBitmap* image)
:	m_text(text)
,	m_checkBox(checkBox)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
{
	setCheckBox(checkBox);
}

MenuItem::MenuItem(const Command& command, const std::wstring& text, IBitmap* image)
:	m_command(command)
,	m_text(text)
,	m_checkBox(false)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
{
}

MenuItem::MenuItem(const std::wstring& text, IBitmap* image)
:	m_text(text)
,	m_checkBox(false)
,	m_image(image)
,	m_enable(true)
,	m_checked(false)
{
}

MenuItem::MenuItem(const Command& command, const std::wstring& text)
:	m_command(command)
,	m_text(text)
,	m_checkBox(false)
,	m_enable(true)
,	m_checked(false)
{
}

MenuItem::MenuItem(const std::wstring& text)
:	m_text(text)
,	m_checkBox(false)
,	m_enable(true)
,	m_checked(false)
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
	if ((m_checkBox = checkBox) == true)
	{
		m_imageUnchecked = new StyleBitmap(L"UI.Unchecked");
		m_imageChecked = new StyleBitmap(L"UI.Checked");
	}
	else
	{
		m_imageUnchecked = nullptr;
		m_imageChecked = nullptr;
	}
}

bool MenuItem::getCheckBox() const
{
	return m_checkBox;
}

const std::wstring& MenuItem::getText() const
{
	return m_text;
}

void MenuItem::setImage(IBitmap* image)
{
	m_image = image;
}

IBitmap* MenuItem::getImage() const
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
	m_items.remove(item);
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
		return nullptr;

	return m_items[index];
}

Size MenuItem::getSize(const Widget* shell) const
{
	//if (m_text != L"-")
	//{
		int32_t cw = shell->getFontMetric().getExtent(m_text).cx;
		int32_t ch = shell->getFontMetric().getHeight();

		if (m_checkBox)
		{
			const Size sz = m_imageUnchecked->getSize(shell);
			cw += shell->pixel(c_itemMarginX) + sz.cx;
			ch = std::max< int32_t >(ch, sz.cy);
		}

		return Size(cw + shell->pixel(c_itemMarginX) * 2, ch + shell->pixel(c_itemMarginY) * 2);
	//}
	//else
	//	return Size(0, 1 + dpi96(c_itemMarginY) * 2);
}

void MenuItem::paint(const Widget* shell, Canvas& canvas, const Rect& rc, bool tracking) const
{
	const StyleSheet* ss = shell->getStyleSheet();
	const Rect rcLabel = rc.inflate(-shell->pixel(c_itemMarginX), 0);

	canvas.setBackground(ss->getColor(this, tracking ? L"background-color-hover" : L"background-color"));
	canvas.fillRect(rc);

	canvas.setForeground(ss->getColor(this, L"color"));

	if (m_text != L"-")
		canvas.drawText(rcLabel, m_text, AnLeft, AnCenter);
	else
		canvas.drawLine(
			Point(rcLabel.left, rcLabel.getCenter().y),
			Point(rcLabel.right, rcLabel.getCenter().y)
		);

	if (m_checkBox)
	{
		IBitmap* image = m_checked ? m_imageChecked : m_imageUnchecked;
		const Size sz = image->getSize(shell);

		canvas.drawBitmap(
			Point(rcLabel.right - sz.cx, rcLabel.top + (rcLabel.getHeight() - sz.cy) / 2),
			Point(0, 0),
			sz,
			image,
			BlendMode::Alpha
		);
	}
}

}
