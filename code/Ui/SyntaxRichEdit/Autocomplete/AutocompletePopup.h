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
#include "Ui/Widget.h"
#include "Ui/SyntaxRichEdit/Autocomplete/IAutocompleteProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#    define T_DLLCLASS T_DLLEXPORT
#else
#    define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class ScrollBar;

/*! Autocomplete popup window.
 * \ingroup UI
 */
class T_DLLCLASS AutocompletePopup : public Widget
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
    std::vector< AutocompleteSuggestion > m_suggestions;
    int32_t m_selectedIndex = 0;
    int32_t m_scrollOffset = 0;
    int32_t m_itemHeight = 20;
    int32_t m_visibleItems = 10;
    Ref< ScrollBar > m_scrollBar;

    void eventPaint(PaintEvent* event);

    void eventSize(SizeEvent* event);

    void eventButtonDown(MouseButtonDownEvent* event);

    void eventMouseMove(MouseMoveEvent* event);

    void updateScrollBar();

    int32_t hitTest(const Point& pt) const;
};

}
