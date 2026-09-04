/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Ref.h"
#include "Ui/ToolForm.h"
#include "Ui/SyntaxRichEdit/Autocomplete/IAutocompleteProvider.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteSelectEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#    define T_DLLCLASS T_DLLEXPORT
#else
#    define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class AutocompleteList;

/*! Autocomplete popup window.
 * \ingroup UI
 */
class T_DLLCLASS AutocompletePopup : public ToolForm
{
    T_RTTI_CLASS;

public:
    AutocompletePopup();

    bool create(Widget* parent);

    void setSuggestions(IAutocompleteProvider* provider, const std::vector< AutocompleteSuggestion >& suggestions);

    void clearSuggestions();

    int32_t getSelectedIndex() const;

    void setSelectedIndex(int32_t index);

    const AutocompleteSuggestion* getSelectedSuggestion() const;

    bool selectNext();

    bool selectPrevious();

    int32_t getSuggestionCount() const;

private:
    Ref< IAutocompleteProvider > m_provider;
    Ref< AutocompleteList > m_list;

    void eventSelect(AutocompleteSelectEvent* event);
};

}
