/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Event.h"
#include "Ui/Autocomplete/IAutocompleteProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#    define T_DLLCLASS T_DLLEXPORT
#else
#    define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Autocomplete selection event.
 * \ingroup UI
 */
class T_DLLCLASS AutocompleteSelectEvent : public Event
{
    T_RTTI_CLASS;

public:
    explicit AutocompleteSelectEvent(EventSubject* sender);

    explicit AutocompleteSelectEvent(EventSubject* sender, const AutocompleteSuggestion& suggestion);

    const AutocompleteSuggestion& getSuggestion() const;

private:
    AutocompleteSuggestion m_suggestion;
};

}
