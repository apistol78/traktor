/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Button.h"
#include "Ui/Edit.h"
#include "Ui/Command.h"
#include "Ui/ComboBox.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ComboBox", ComboBox, Widget)

bool ComboBox::create(Widget* parent, const std::wstring& text, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_buttonArrow = new Button();
	m_buttonArrow->create(this, L"...");

	m_edit = new Edit();
	m_edit->create(this, text, WsNone);

	m_listBox = new ListBox();
	m_listBox->create(this);

	addEventHandler< SizeEvent >(this, &ComboBox::eventSize);
	return true;
}

int ComboBox::add(const std::wstring& item)
{
	return m_listBox->add(item);
}

bool ComboBox::remove(int index)
{
	return m_listBox->remove(index);
}

void ComboBox::removeAll()
{
	m_listBox->removeAll();
}

int ComboBox::count() const
{
	return m_listBox->count();
}

std::wstring ComboBox::get(int index) const
{
	return m_listBox->getItem(index);
}

void ComboBox::select(int index)
{
}

int ComboBox::getSelected() const
{
	return -1;
}

std::wstring ComboBox::getSelectedItem() const
{
	return std::wstring();
}

void ComboBox::eventSize(SizeEvent* event)
{
	const Size sz = event->getSize();
	m_edit->setRect(Rect(Point(0, 0), Size(sz.cx - 16, 16)));
	m_buttonArrow->setRect(Rect(Point(sz.cx - 16, 0), Size(16, 16)));
	m_listBox->setRect(Rect(Point(0, 16), Size(sz.cx, sz.cy - 16)));
}

}
