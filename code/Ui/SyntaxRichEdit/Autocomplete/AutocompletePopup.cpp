/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompletePopup.h"

#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleSheet.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteList.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteItem.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteSelectEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutocompletePopup", AutocompletePopup, ToolForm)

AutocompletePopup::AutocompletePopup()
{
}

bool AutocompletePopup::create(Widget* parent)
{
	// Create as a borderless, non-resizable tool form with flood layout
	if (!ToolForm::create(parent, L"", Unit(400), Unit(200), WsNone, new FloodLayout()))
		return false;

	// Create autocomplete list to display suggestions
	m_list = new AutocompleteList();
	m_list->create(this);
	m_list->addEventHandler< AutocompleteSelectEvent >(this, &AutocompletePopup::eventSelect);

	return true;
}

void AutocompletePopup::setSuggestions(IAutocompleteProvider* provider, const std::vector< AutocompleteSuggestion >& suggestions)
{
	m_provider = provider;

	// Clear and repopulate list
	m_list->removeAll();
	for (const auto& suggestion : suggestions)
	{
		// Get icon for this suggestion type
		Ref< IBitmap > icon = provider ? provider->getSymbolIcon(suggestion.type) : nullptr;

		// Create item cell
		Ref< AutocompleteItem > item = new AutocompleteItem(suggestion, icon);
		m_list->addItem(item);
	}

	// Select first item
	if (!suggestions.empty())
		m_list->setSelectedIndex(0);

	// Force immediate layout and repaint
	m_list->updateLayout();
	update();
}

void AutocompletePopup::clearSuggestions()
{
	m_list->removeAll();
}

int32_t AutocompletePopup::getSelectedIndex() const
{
	return m_list->getSelectedIndex();
}

void AutocompletePopup::setSelectedIndex(int32_t index)
{
	m_list->setSelectedIndex(index);
}

const AutocompleteSuggestion* AutocompletePopup::getSelectedSuggestion() const
{
	const AutocompleteItem* item = m_list->getSelectedItem();
	if (item)
		return &item->getSuggestion();
	return nullptr;
}

bool AutocompletePopup::selectNext()
{
	return m_list->selectNext();
}

bool AutocompletePopup::selectPrevious()
{
	return m_list->selectPrevious();
}

int32_t AutocompletePopup::getSuggestionCount() const
{
	return m_list->count();
}

void AutocompletePopup::eventSelect(AutocompleteSelectEvent* event)
{
	// When user clicks/selects an item in the list, forward the event
	raiseEvent(event);
}

}
