/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include <map>
#include "Core/Containers/AlignedVector.h"
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

class IBitmap;

/*! Symbol information for caching.
 * \ingroup UI
 */
struct SymbolInfo
{
    std::wstring name;
    SymbolType type;
    std::wstring description;
    int32_t line = -1;
    int32_t scope = 0;                //!< Scope depth (0 = global, higher = deeper local scope)
    bool isLocal = false;

    SymbolInfo() = default;
    SymbolInfo(const std::wstring& name_, SymbolType type_, int32_t line_ = -1, int32_t scope_ = 0, bool isLocal_ = false)
        : name(name_), type(type_), line(line_), scope(scope_), isLocal(isLocal_) {}
};

/*! Lua language autocomplete provider.
 * \ingroup UI
 */
class T_DLLCLASS LuaAutocompleteProvider : public IAutocompleteProvider
{
    T_RTTI_CLASS;

public:
    LuaAutocompleteProvider();

    virtual bool getSuggestions(const AutocompleteContext& context, std::vector< AutocompleteSuggestion >& outSuggestions) override;

    virtual void updateContent(const std::wstring& text, const std::wstring& fileName = L"") override;

    virtual Ref< IBitmap > getSymbolIcon(SymbolType type) const override;

private:
    AlignedVector< SymbolInfo > m_symbols;
    std::map< std::wstring, std::set< std::wstring > > m_tableMembers;
    mutable std::map< SymbolType, Ref< IBitmap > > m_iconCache;

    void parseSymbols(const std::wstring& text);

    void addLuaBuiltins();

    void addKeywords();

    bool matchesPrefix(const std::wstring& symbol, const std::wstring& prefix) const;

    int32_t calculateRelevance(const SymbolInfo& symbol, const AutocompleteContext& context) const;

    std::wstring extractCurrentScope(const std::wstring& text, int32_t caretOffset) const;
};

}
