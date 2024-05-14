/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/ListBox/ListBoxItem.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

struct ListBoxItemPred
{
	bool operator () (const ListBoxItem* lh, const ListBoxItem* rh) const
	{
		return compareIgnoreCase(lh->getText(), rh->getText()) < 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListBox", ListBox, AutoWidget)

ListBox::ListBox()
:	m_style(0)
,	m_lastHitIndex(-1)
{
}

bool ListBox::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style | WsDoubleBuffer))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ListBox::eventButtonDown);
	addEventHandler< KeyDownEvent >(this, &ListBox::eventKeyDown);

	m_style = style;
	m_lastHitIndex = -1;
	return true;
}

Size ListBox::getPreferredSize(const Size& hint) const
{
	return Size(100, 100);
}

int32_t ListBox::add(const std::wstring& item, Object* data)
{
	Ref< ListBoxItem > lbi = new ListBoxItem();
	lbi->setText(item);
	lbi->setData(L"DATA", data);
	m_items.push_back(lbi);
	m_lastHitIndex = -1;
	requestUpdate();
	return int32_t(m_items.size() - 1);
}

int32_t ListBox::add(const std::wstring& item, const Color4ub& bgcolor, Object* data)
{
	Ref< ListBoxItem > lbi = new ListBoxItem();
	lbi->setText(item);
	lbi->setBackgroundColor(bgcolor);
	lbi->setData(L"DATA", data);
	m_items.push_back(lbi);
	m_lastHitIndex = -1;
	requestUpdate();
	return int32_t(m_items.size() - 1);
}

bool ListBox::remove(int32_t index)
{
	if (index >= 0 && index < m_items.size())
	{
		m_items.erase(m_items.begin() + index);
		m_lastHitIndex = -1;
		requestUpdate();
		return true;
	}
	else
		return false;
}

void ListBox::removeAll()
{
	m_items.clear();
	m_lastHitIndex = -1;
	requestUpdate();
}

int32_t ListBox::count() const
{
	return (int32_t)m_items.size();
}

void ListBox::setItem(int32_t index, const std::wstring& item)
{
	if (index >= 0 && index < (int32_t)m_items.size())
	{
		m_items[index]->setText(item);
		requestUpdate();
	}
}

void ListBox::setData(int32_t index, Object* data)
{
	if (index >= 0 && index < (int32_t)m_items.size())
		m_items[index]->setData(L"DATA", data);
}

std::wstring ListBox::getItem(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_items.size())
		return m_items[index]->getText();
	else
		return L"";
}

Object* ListBox::getData(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_items.size())
		return m_items[index]->getData(L"DATA");
	else
		return nullptr;
}

void ListBox::select(int32_t index)
{
	if (index >= 0 && index < (int32_t)m_items.size())
		m_items[index]->setSelected(true);
}

bool ListBox::selected(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_items.size())
		return m_items[index]->isSelected();
	else
		return false;
}

int32_t ListBox::getSelected(std::vector< int32_t >& selected) const
{
	selected.resize(0);
	for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
	{
		if (m_items[i]->isSelected())
			selected.push_back(i);
	}
	return int32_t(selected.size());
}

int32_t ListBox::getSelected() const
{
	for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
	{
		if (m_items[i]->isSelected())
			return i;
	}
	return -1;
}

std::wstring ListBox::getSelectedItem() const
{
	int32_t index = getSelected();
	if (index >= 0)
		return m_items[index]->getText();
	else
		return L"";
}

Object* ListBox::getSelectedData() const
{
	int32_t index = getSelected();
	if (index >= 0)
		return m_items[index]->getData(L"DATA");
	else
		return nullptr;
}

int32_t ListBox::getItemHeight()
{
	return getFontMetric().getHeight() + pixel(6_ut);
}

Rect ListBox::getItemRect(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_items.size())
		return m_items[index]->getRect();
	else
		return Rect();
}

void ListBox::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	Ref< ListBoxItem > hitItem = dynamic_type_cast< ListBoxItem* >(hitTest(event->getPosition()));

	int32_t hitIndex = -1;
	for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
	{
		if (hitItem == m_items[i])
		{
			hitIndex = i;
			break;
		}
	}
	if (hitIndex < 0)
		return;

	bool modified = false;

	if ((m_style & (WsMultiple | WsExtended)) == WsMultiple)
	{
		if (hitItem)
			modified |= hitItem->setSelected(!hitItem->isSelected());
	}
	else if ((m_style & WsExtended) == WsExtended)
	{
		if ((event->getKeyState() & KsControl) == KsControl)
			modified |= hitItem->setSelected(!hitItem->isSelected());
		else if ((event->getKeyState() & KsShift) == KsShift)
		{
			if (m_lastHitIndex >= 0)
			{
				for (int32_t i = std::min(m_lastHitIndex, hitIndex); i <= std::max(m_lastHitIndex, hitIndex); ++i)
					modified |= m_items[i]->setSelected(true);
			}
		}
		else
		{
			for (auto item : m_items)
				modified |= item->setSelected(hitItem == item);
		}
	}
	else
	{
		for (auto item : m_items)
			modified |= item->setSelected(hitItem == item);
	}

	if (modified)
	{
		SelectionChangeEvent selectionChangeEvent(this);
		raiseEvent(&selectionChangeEvent);
	}

	m_lastHitIndex = hitIndex;
}

void ListBox::eventKeyDown(KeyDownEvent* event)
{
	int32_t index = getSelected();
	switch (event->getVirtualKey())
	{
	case VkUp:
		if (index > 0)
			index--;
		break;

	case VkDown:
		if (index < count() - 1)
			index++;
		break;

	default:
		break;
	}
	if (index != getSelected())
	{
		for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
			m_items[i]->setSelected(i == index);

		SelectionChangeEvent selectionChangeEvent(this);
		raiseEvent(&selectionChangeEvent);
	}
}

void ListBox::layoutCells(const Rect& rc)
{
	int32_t height = getItemHeight();

	Rect rcItem(rc.left, rc.top, rc.right, rc.top + height);
	if ((m_style & WsSort) == WsSort)
	{
		RefArray< ListBoxItem > items = m_items;
		items.sort(ListBoxItemPred());
		for (uint32_t i = 0; i < items.size(); ++i)
		{
			placeCell(items[i], rcItem);
			rcItem = rcItem.offset(0, height);
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_items.size(); ++i)
		{
			placeCell(m_items[i], rcItem);
			rcItem = rcItem.offset(0, height);
		}
	}
}

	}
}
