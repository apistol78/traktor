/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Misc/String.h"
#include "Script/Editor/AutocompleteProviderLua.h"
#include "Script/Editor/IScriptOutline.h"
#include "Script/Editor/ScriptOutlineLua.h"
#include "Ui/StyleBitmap.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.AutocompleteProviderLua", AutocompleteProviderLua, ui::IAutocompleteProvider)

AutocompleteProviderLua::AutocompleteProviderLua(IScriptOutline* outline)
:	m_outline(outline)
,	m_preprocessor(new Preprocessor())
{
    addLuaBuiltins();
    addKeywords();
}

bool AutocompleteProviderLua::getSuggestions(const ui::AutocompleteContext& context, std::vector< ui::AutocompleteSuggestion >& outSuggestions)
{
    const std::wstring& prefix = context.currentWord;
    if (prefix.empty())
        return false;

    // Collect matching symbols
    std::vector< std::pair< SymbolInfo, int32_t > > candidates;

    for (const auto& symbol : m_symbols)
    {
        if (matchesPrefix(symbol.name, prefix))
        {
            int32_t relevance = calculateRelevance(symbol, context);
            candidates.push_back(std::make_pair(symbol, relevance));
        }
    }

    // Sort by relevance (highest first)
    std::sort(candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

    // Convert to suggestions (limit to reasonable number)
    const size_t maxSuggestions = 50;
    outSuggestions.reserve(std::min(candidates.size(), maxSuggestions));

    for (size_t i = 0; i < std::min(candidates.size(), maxSuggestions); ++i)
    {
        const auto& [symbol, relevance] = candidates[i];
        outSuggestions.push_back(ui::AutocompleteSuggestion(symbol.name, symbol.description, symbol.type, relevance));
    }

    return !outSuggestions.empty();
}

void AutocompleteProviderLua::updateContent(const std::wstring& text, const std::wstring& fileName)
{
    // Clear existing user symbols (keep builtins and keywords)
    m_symbols.erase(
        std::remove_if(m_symbols.begin(), m_symbols.end(),
            [](const SymbolInfo& symbol) {
		return symbol.type != ui::SymbolType::Keyword &&
                       symbol.scope >= 0; // User symbols have scope >= 0, builtins have -1
            }),
        m_symbols.end());

    m_tableMembers.clear();
    parseSymbols(text);
}

Ref< ui::IBitmap > AutocompleteProviderLua::getSymbolIcon(ui::SymbolType type) const
{
    auto it = m_iconCache.find(type);
    if (it != m_iconCache.end())
        return it->second;

    Ref< ui::IBitmap > icon;
    switch (type)
    {
	case ui::SymbolType::Function:
		icon = new ui::StyleBitmap(L"Script.DefineGlobalFunction");
        break;
	case ui::SymbolType::Variable:
		icon = new ui::StyleBitmap(L"Script.Variable");
        break;
	case ui::SymbolType::Keyword:
		icon = new ui::StyleBitmap(L"Script.Keyword");
        break;
	case ui::SymbolType::Module:
		icon = new ui::StyleBitmap(L"Script.Module");
        break;
	case ui::SymbolType::Field:
		icon = new ui::StyleBitmap(L"Script.Field");
        break;
	case ui::SymbolType::Parameter:
		icon = new ui::StyleBitmap(L"Script.Parameter");
        break;
	case ui::SymbolType::Constant:
		icon = new ui::StyleBitmap(L"Script.Constant");
        break;
    }

    m_iconCache[type] = icon;
    return icon;
}

void AutocompleteProviderLua::parseSymbols(const std::wstring& text)
{
    if (!m_outline)
        return;

    // Preprocess the script text (handle #if, #define, etc.)
    std::wstring preprocessedText;
    std::set< std::wstring > usings;
    if (m_preprocessor)
    {
        if (!m_preprocessor->evaluate(text, preprocessedText, usings))
            return;
    }
    else
    {
        preprocessedText = text;
    }

    Ref< IScriptOutline::Node > root = m_outline->parse(preprocessedText);
    if (!root)
        return;

    // Walk the outline tree to extract symbols
    std::map< std::wstring, int32_t > functionReferences;
    walkOutlineTree(root, 0, functionReferences);

    // Boost relevance of frequently referenced functions
    for (auto& symbol : m_symbols)
    {
        if (symbol.type == ui::SymbolType::Function)
        {
            auto it = functionReferences.find(symbol.name);
            if (it != functionReferences.end())
            {
                symbol.referenceCount = it->second;
            }
        }
    }
}

void AutocompleteProviderLua::walkOutlineTree(IScriptOutline::Node* node, int32_t scopeDepth, std::map< std::wstring, int32_t >& functionReferences)
{
    while (node)
    {
        // Handle function definitions
        if (auto funcNode = dynamic_type_cast< IScriptOutline::FunctionNode* >(node))
        {
            const std::wstring& name = funcNode->getName();
            if (!name.empty())
            {
                SymbolInfo funcSymbol(
                    name,
                    ui::SymbolType::Function,
                    funcNode->getLine(),
                    scopeDepth,
                    funcNode->isLocal()
                );
                funcSymbol.description = funcNode->isLocal() ? L"local function" : L"function";
                m_symbols.push_back(funcSymbol);
            }

            // Recurse into function body (deeper scope)
            if (funcNode->getBody())
            {
                walkOutlineTree(funcNode->getBody(), scopeDepth + 1, functionReferences);
            }
        }
        // Handle function references (calls)
        else if (auto refNode = dynamic_type_cast< IScriptOutline::FunctionReferenceNode* >(node))
        {
            const std::wstring& name = refNode->getName();
            if (!name.empty())
            {
                functionReferences[name]++;
            }
        }
        // Handle variable declarations
        else if (auto varNode = dynamic_type_cast< VariableNode* >(node))
        {
            const std::wstring& name = varNode->getName();
            if (!name.empty())
            {
                SymbolInfo varSymbol(
                    name,
                    ui::SymbolType::Variable,
                    varNode->getLine(),
                    scopeDepth,
                    varNode->isLocal()
                );
                varSymbol.description = varNode->isLocal() ? L"local variable" : L"variable";
                m_symbols.push_back(varSymbol);
            }
        }
        // Handle table field access
        else if (auto fieldNode = dynamic_type_cast< FieldAccessNode* >(node))
        {
            const std::wstring& tableName = fieldNode->getTableName();
            const std::wstring& fieldName = fieldNode->getFieldName();
            if (!tableName.empty() && !fieldName.empty())
            {
                // Track which fields are accessed on which tables
                m_tableMembers[tableName].insert(fieldName);
            }
        }

        // Move to next sibling node
        node = node->getNext();
    }
}

void AutocompleteProviderLua::addLuaBuiltins()
{
    // Standard Lua global functions
    const std::vector< std::pair< std::wstring, std::wstring > > luaBuiltins = {
        { L"assert", L"assert(v [, message]) - Issues an error when the value v is false" },
        { L"collectgarbage", L"collectgarbage([opt [, arg]]) - Generic interface to the garbage collector" },
        { L"dofile", L"dofile([filename]) - Opens file and executes its contents" },
        { L"error", L"error(message [, level]) - Terminates with an error" },
        { L"getmetatable", L"getmetatable(object) - Returns the metatable of the given object" },
        { L"ipairs", L"ipairs(t) - Returns iterator function for array part of table" },
        { L"load", L"load(chunk [, chunkname [, mode [, env]]]) - Loads a chunk" },
        { L"loadfile", L"loadfile([filename [, mode [, env]]]) - Loads a file as a chunk" },
        { L"next", L"next(table [, index]) - Returns next index and value pair" },
        { L"pairs", L"pairs(t) - Returns iterator function for table" },
        { L"pcall", L"pcall(f [, arg1, ...]) - Calls function in protected mode" },
        { L"print", L"print(...) - Prints values to standard output" },
        { L"rawequal", L"rawequal(v1, v2) - Checks equality without metamethods" },
        { L"rawget", L"rawget(table, index) - Gets value without metamethods" },
        { L"rawlen", L"rawlen(v) - Returns length without metamethods" },
        { L"rawset", L"rawset(table, index, value) - Sets value without metamethods" },
        { L"require", L"require(modname) - Loads and returns a module" },
        { L"select", L"select(index, ...) - Returns arguments from index" },
        { L"setmetatable", L"setmetatable(table, metatable) - Sets table's metatable" },
        { L"tonumber", L"tonumber(e [, base]) - Converts to number" },
        { L"tostring", L"tostring(v) - Converts to string" },
        { L"type", L"type(v) - Returns type of value as string" },
        { L"xpcall", L"xpcall(f, msgh [, arg1, ...]) - Calls function with message handler" }
    };

    for (const auto& [name, desc] : luaBuiltins)
    {
		SymbolInfo symbol(name, ui::SymbolType::Function, -1, -1, false);
        symbol.description = desc;
        m_symbols.push_back(symbol);
    }

    // Standard library tables
    const std::vector< std::wstring > luaLibraries = {
        L"string", L"table", L"math", L"io", L"os", L"debug", L"coroutine", L"package", L"utf8"
    };

    for (const auto& lib : luaLibraries)
    {
		SymbolInfo symbol(lib, ui::SymbolType::Module, -1, -1, false);
        symbol.description = L"Standard library";
        m_symbols.push_back(symbol);
    }

    // Constants
    const std::vector< std::pair< std::wstring, std::wstring > > luaConstants = {
        { L"nil", L"Represents no value" },
        { L"true", L"Boolean true value" },
        { L"false", L"Boolean false value" },
        { L"_G", L"Global environment table" },
        { L"_VERSION", L"Lua version string" }
    };

    for (const auto& [name, desc] : luaConstants)
    {
		SymbolInfo symbol(name, ui::SymbolType::Constant, -1, -1, false);
        symbol.description = desc;
        m_symbols.push_back(symbol);
    }
}

void AutocompleteProviderLua::addKeywords()
{
    const std::vector< std::wstring > luaKeywords = {
        L"and", L"break", L"do", L"else", L"elseif", L"end", L"false", L"for",
        L"function", L"if", L"in", L"local", L"nil", L"not", L"or", L"repeat",
        L"return", L"then", L"true", L"until", L"while"
    };

    for (const auto& keyword : luaKeywords)
    {
        m_symbols.push_back(SymbolInfo(keyword, ui::SymbolType::Keyword, -1, -1, false));
    }
}

bool AutocompleteProviderLua::matchesPrefix(const std::wstring& symbol, const std::wstring& prefix) const
{
    if (prefix.length() > symbol.length())
        return false;

    // Case-insensitive prefix match
    for (size_t i = 0; i < prefix.length(); ++i)
    {
        if (towlower(symbol[i]) != towlower(prefix[i]))
            return false;
    }

    return true;
}

int32_t AutocompleteProviderLua::calculateRelevance(const SymbolInfo& symbol, const ui::AutocompleteContext& context) const
{
    int32_t relevance = 0;

    // Exact match gets highest priority
    if (symbol.name == context.currentWord)
        relevance += 1000;

    // Prefix length match (longer prefix = higher relevance)
    if (symbol.name.length() >= context.currentWord.length())
        relevance += 100 - (int32_t)(symbol.name.length() - context.currentWord.length());

    // Symbol type relevance
    switch (symbol.type)
    {
	case ui::SymbolType::Function:
        relevance += 50;
        break;
	case ui::SymbolType::Variable:
        relevance += 40;
        break;
	case ui::SymbolType::Field:
        relevance += 30;
        break;
	case ui::SymbolType::Keyword:
        relevance += 20;
        break;
	case ui::SymbolType::Constant:
        relevance += 15;
        break;
    default:
        relevance += 10;
        break;
    }

    // Local symbols get higher relevance when in appropriate scope
    if (symbol.isLocal && symbol.scope <= context.line)
        relevance += 25;

    // Recently defined symbols get bonus
    if (symbol.line >= 0 && context.line >= 0)
    {
        int32_t distance = abs(context.line - symbol.line);
        if (distance < 10)
            relevance += 10 - distance;
    }

    // Boost frequently referenced functions
    if (symbol.referenceCount > 0)
        relevance += std::min(symbol.referenceCount * 5, 50);

    return relevance;
}

std::wstring AutocompleteProviderLua::extractCurrentScope(const std::wstring& text, int32_t caretOffset) const
{
    // Find the current function or block scope
    // This is a simplified implementation
    int32_t pos = caretOffset;
    while (pos > 0 && text[pos] != L'\n')
        pos--;

    return text.substr(pos, caretOffset - pos);
}

}
