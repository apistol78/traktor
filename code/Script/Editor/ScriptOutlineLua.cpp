/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstdlib>
#include <map>
#include "Core/Log/Log.h"
#include "Script/Editor/Lexer.h"
#include "Script/Editor/LuaGrammar.h"
#include "Script/Editor/LuaGrammarToken.h"
#include "Script/Editor/ScriptOutlineLua.h"

// Generated parser interface.
void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *p, void (*freeProc)(void*));
void Parse(void *yyp, int yymajor, traktor::script::LuaGrammarToken* yyminor, const traktor::script::ScriptOutlineLua* outline);

namespace traktor::script
{
	namespace
	{

const std::map< wchar_t, int32_t >& getSingles()
{
	static std::map< wchar_t, int32_t > single;
	if (single.empty())
	{
		single[L'('] = TOKEN_LEFT_PARANTHESIS;
		single[L';'] = TOKEN_SEMICOLON;
		single[L':'] = TOKEN_COLON;
		single[L','] = TOKEN_COMMA;
		single[L'+'] = TOKEN_PLUS;
		single[L'-'] = TOKEN_MINUS;
		single[L'*'] = TOKEN_MULTIPLY;
		single[L'/'] = TOKEN_DIVIDE;
		single[L'%'] = TOKEN_MODULO;
		single[L'#'] = TOKEN_HASH;
		single[L'^'] = TOKEN_RAISE;
		single[L'['] = TOKEN_LEFT_BRACKET;
		single[L']'] = TOKEN_RIGHT_BRACKET;
		single[L')'] = TOKEN_RIGHT_PARANTHESIS;
		single[L'{'] = TOKEN_LEFT_BRACE;
		single[L'}'] = TOKEN_RIGHT_BRACE;
		single[L'|'] = TOKEN_BITWISE_OR;
		single[L'&'] = TOKEN_BITWISE_AND;
	}
	return single;
}

const std::map< std::wstring, int32_t >& getTuples()
{
	static std::map< std::wstring, int32_t > tuples;
	if (tuples.empty())
	{
		tuples[L"const"] = TOKEN_CONST;
		tuples[L"until"] = TOKEN_UNTIL;
		tuples[L"do"] = TOKEN_DO;
		tuples[L"end"] = TOKEN_END;
		tuples[L"while"] = TOKEN_WHILE;
		tuples[L"repeat"] = TOKEN_REPEAT;
		tuples[L"if"] = TOKEN_IF;
		tuples[L"function"] = TOKEN_FUNCTION;
		tuples[L"for"] = TOKEN_FOR;
		tuples[L"in"] = TOKEN_IN;
		tuples[L"else"] = TOKEN_ELSE;
		tuples[L"elseif"] = TOKEN_ELSEIF;
		tuples[L"then"] = TOKEN_THEN;
		tuples[L"break"] = TOKEN_BREAK;
		tuples[L"return"] = TOKEN_RETURN;
		tuples[L"local"] = TOKEN_LOCAL;
		tuples[L"or"] = TOKEN_OR;
		tuples[L"and"] = TOKEN_AND;
		tuples[L"<"] = TOKEN_LESS;
		tuples[L">"] = TOKEN_GREATER;
		tuples[L"<="] = TOKEN_LESS_EQUAL;
		tuples[L">="] = TOKEN_GREATER_EQUAL;
		tuples[L"="] = TOKEN_EQUAL;
		tuples[L"=="] = TOKEN_EQUAL_EQUAL;
		tuples[L"~="] = TOKEN_NOT_EQUAL;
		tuples[L"not"] = TOKEN_NOT;
		tuples[L"nil"] = TOKEN_NIL;
		tuples[L"true"] = TOKEN_TRUE;
		tuples[L"false"] = TOKEN_FALSE;
		tuples[L"."] = TOKEN_DOT;
		tuples[L".."] = TOKEN_DOT_DOT;
		tuples[L"..."] = TOKEN_DOT_DOT_DOT;
	}
	return tuples;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptOutlineLua", 0, ScriptOutlineLua, IScriptOutline)

Ref< IScriptOutline::Node > ScriptOutlineLua::parse(const std::wstring& text) const
{
	Lexer lx(text.c_str(), (uint32_t)text.length());

	void* parser = ParseAlloc(malloc);
	if (!parser)
		return nullptr;

	const auto& single = getSingles();
	const auto& tuples = getTuples();

	int32_t type;
	while ((type = lx.next()) != Lexer::LtEndOfFile)
	{
		if (type == Lexer::LtWord)
		{
			const std::wstring word = lx.getWord();
			const auto it = tuples.find(word);
			if (it != tuples.end())
				Parse(parser, it->second, 0, this);
			else
				Parse(parser, TOKEN_NAME, new LuaGrammarToken(word, lx.getLine()), this);
		}
		else if (type == Lexer::LtString)
		{
			const std::wstring str = lx.getString();
			Parse(parser, TOKEN_STRING, new LuaGrammarToken(str, lx.getLine()), this);
		}
		else if (type == Lexer::LtNumber)
		{
			const double number = lx.getNumber();
			Parse(parser, TOKEN_NUMBER, new LuaGrammarToken(L"", lx.getLine()), this);
		}
		else
		{
			const auto it = single.find(type);
			if (it != single.end())
				Parse(parser, it->second, new LuaGrammarToken(L"", lx.getLine()), this);
		}
	}

	Parse(parser, 0, nullptr, this);
	ParseFree(parser, free);

	return m_result;
}

void ScriptOutlineLua::parseResult(Node* node) const
{
	m_result = node;
}

void ScriptOutlineLua::syntaxError() const
{
}

}
