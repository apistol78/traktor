/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteSelectEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutocompleteSelectEvent", AutocompleteSelectEvent, Event)

AutocompleteSelectEvent::AutocompleteSelectEvent(EventSubject* sender)
:    Event(sender)
{
}

AutocompleteSelectEvent::AutocompleteSelectEvent(EventSubject* sender, const AutocompleteSuggestion& suggestion)
:    Event(sender)
,    m_suggestion(suggestion)
{
}

const AutocompleteSuggestion& AutocompleteSelectEvent::getSuggestion() const
{
    return m_suggestion;
}

}
