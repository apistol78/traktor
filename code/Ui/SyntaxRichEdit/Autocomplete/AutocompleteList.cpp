/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteList.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteItem.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteSelectEvent.h"
#include "Ui/Application.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutocompleteList", AutocompleteList, AutoWidget)

bool AutocompleteList::create(Widget* parent)
{
	if (!AutoWidget::create(parent, WsFocus | WsDoubleBuffer))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &AutocompleteList::eventButtonDown);
	return true;
}

void AutocompleteList::addItem(AutocompleteItem* item)
{
	m_items.push_back(item);
	requestUpdate();
}

void AutocompleteList::removeAll()
{
	m_items.clear();
	requestUpdate();
}

int32_t AutocompleteList::count() const
{
	return (int32_t)m_items.size();
}

AutocompleteItem* AutocompleteList::getItem(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_items.size())
		return m_items[index];
	return nullptr;
}

AutocompleteItem* AutocompleteList::getSelectedItem() const
{
	for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
	{
		if (m_items[i]->isSelected())
			return m_items[i];
	}
	return nullptr;
}

int32_t AutocompleteList::getSelectedIndex() const
{
	for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
	{
		if (m_items[i]->isSelected())
			return i;
	}
	return -1;
}

void AutocompleteList::setSelectedIndex(int32_t index)
{
	// Deselect all items first
	for (int32_t i = 0; i < (int32_t)m_items.size(); ++i)
		m_items[i]->setSelected(false);

	// Select the specified item
	if (index >= 0 && index < (int32_t)m_items.size())
		m_items[index]->setSelected(true);

	requestUpdate();
}

bool AutocompleteList::selectNext()
{
	const int32_t current = getSelectedIndex();
	if (current < (int32_t)m_items.size() - 1)
	{
		setSelectedIndex(current + 1);
		return true;
	}
	return false;
}

bool AutocompleteList::selectPrevious()
{
	const int32_t current = getSelectedIndex();
	if (current > 0)
	{
		setSelectedIndex(current - 1);
		return true;
	}
	return false;
}

void AutocompleteList::layoutCells(const Rect& rc)
{
	// Each item should be about 24 pixels high (matching the design in AutocompleteItem::paint)
	const int32_t itemHeight = getFontMetric().getHeight() + pixel(4_ut);

	Rect rcItem(rc.left, rc.top, rc.right, rc.top + itemHeight);
	for (uint32_t i = 0; i < m_items.size(); ++i)
	{
		placeCell(m_items[i], rcItem);
		rcItem = rcItem.offset(0, itemHeight);
	}
}

void AutocompleteList::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	// Hit test to find which item was clicked
	AutocompleteItem* hitItem = dynamic_type_cast< AutocompleteItem* >(hitTest(event->getPosition()));
	if (!hitItem)
		return;

	// Find the index of the hit item
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

	// Select the item
	setSelectedIndex(hitIndex);

	// Raise autocomplete select event
	AutocompleteSelectEvent selectEvent(this, hitItem->getSuggestion());
	raiseEvent(&selectEvent);
}

}
