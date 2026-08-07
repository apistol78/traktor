/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#    define T_DLLCLASS T_DLLEXPORT
#else
#    define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;

/*! Symbol type enumeration for autocomplete suggestions. */
enum class SymbolType
{
    Variable,
    Function,
    Keyword,
    Module,
    Field,
    Parameter,
    Constant
};

/*! Autocomplete suggestion item.
 * \ingroup UI
 */
struct T_DLLCLASS AutocompleteSuggestion
{
    std::wstring name;                //!< Symbol name
    std::wstring description;        //!< Optional description or signature
    SymbolType type;                //!< Symbol type
    int32_t priority = 0;            //!< Suggestion priority (higher = more relevant)

    AutocompleteSuggestion() = default;

    AutocompleteSuggestion(const std::wstring& name_, SymbolType type_, int32_t priority_ = 0)
        : name(name_), type(type_), priority(priority_) {}

    AutocompleteSuggestion(const std::wstring& name_, const std::wstring& description_, SymbolType type_, int32_t priority_ = 0)
        : name(name_), description(description_), type(type_), priority(priority_) {}
};

/*! Context information for autocomplete requests.
 * \ingroup UI
 */
struct T_DLLCLASS AutocompleteContext
{
    std::wstring text;                //!< Full text content
    int32_t caretOffset;            //!< Current caret position
    std::wstring currentWord;        //!< Partial word being typed
    bool afterDot = false;            //!< True if triggered after '.'
    int32_t line;                    //!< Current line number
    int32_t column;                    //!< Current column
};

/*! Interface for autocomplete providers.
 * \ingroup UI
 */
class T_DLLCLASS IAutocompleteProvider : public Object
{
    T_RTTI_CLASS;

public:
    /*! Get autocomplete suggestions for given context.
     * \param context Context information about current editing state
     * \param outSuggestions Output vector to populate with suggestions
     * \return True if provider handled the request
     */
    virtual bool getSuggestions(const AutocompleteContext& context, std::vector< AutocompleteSuggestion >& outSuggestions) = 0;

    /*! Update provider with new text content.
     * \param text Full text content for analysis
     * \param fileName Optional file name for context
     */
    virtual void updateContent(const std::wstring& text, const std::wstring& fileName = L"") = 0;

    /*! Get icon for symbol type.
     * \param type Symbol type
     * \return Icon bitmap or null
     */
    virtual Ref< IBitmap > getSymbolIcon(SymbolType type) const = 0;
};

}
