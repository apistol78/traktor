/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Misc/String.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageLua.h"

namespace traktor::ui
{
	namespace
	{

bool match(const std::wstring_view& text, const std::wstring_view& patt)
{
	if (text.length() < patt.length())
		return false;

	for (size_t i = 0; i < patt.length(); ++i)
	{
		if (text[i] != patt[i])
			return false;
	}

	return true;
}

class SyntaxLanguageContextLua : public RefCountImpl< SyntaxLanguage::IContext >
{
public:
	bool m_blockComment = false;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.SyntaxLanguageLua", 0, SyntaxLanguageLua, SyntaxLanguage)

std::wstring SyntaxLanguageLua::lineComment() const
{
	return L"--";
}

Ref< SyntaxLanguage::IContext > SyntaxLanguageLua::createContext() const
{
	return new SyntaxLanguageContextLua();
}

bool SyntaxLanguageLua::consume(SyntaxLanguage::IContext* context, const std::wstring& text, State& outState, int& outConsumedChars) const
{
	SyntaxLanguageContextLua* cx = static_cast< SyntaxLanguageContextLua* >(context);
	const int32_t ln = (int32_t)text.length();
	T_ASSERT(ln > 0);

	if (cx->m_blockComment)
	{
		if (match(text, L"]]"))
		{
			cx->m_blockComment = false;

			outState = StBlockComment;
			outConsumedChars = 2;
			return true;
		}
		else
		{
			outState = StBlockComment;
			outConsumedChars = 1;
			return true;
		}
	}

	// Beginning of comment.
	if (match(text, L"--"))
	{
		if (match(text, L"--[["))
		{
			cx->m_blockComment = true;

			outState = StBlockComment;
			outConsumedChars = 4;
			return true;
		}

		outState = StLineComment;
		outConsumedChars = 2;

		for (int32_t i = 2; i < ln; ++i)
		{
			if (text[i] == L'\n' || text[i] == L'\r')
				break;
			++outConsumedChars;
		}

		return true;
	}

	// Preprocessor
	if (text[0] == L'#')
	{
		outState = StPreprocessor;
		outConsumedChars = 1;
		for (int32_t i = 1; i < ln; ++i)
		{
			if (text[i] == L'\n' || text[i] == L'\r')
				break;
			++outConsumedChars;
		}
		return true;
	}

	// String
	if (text[0] == L'\"')
	{
		outState = StString;
		outConsumedChars = 1;

		for (int i = 1; i < ln; ++i)
		{
			++outConsumedChars;
			if (text[i] == L'\"')
				break;
		}

		return true;
	}

	// Number
	if (text[0] >= L'0' && text[0] <= L'9')
	{
		outState = StNumber;
		outConsumedChars = 1;

		int i = 1;

		// Integer or float.
		for (; (i < ln && (text[i] >= L'0' && text[i] <= L'9')) || text[i] == L'.'; ++i)
			++outConsumedChars;

		// Fractional
		if (text[i] == L'.')
		{
			for (; i < ln && text[i] >= L'0' && text[i] <= L'9'; ++i)
				++outConsumedChars;
		}

		return true;
	}

	// White space.
	if (text[0] == L' ' || text[0] == L'\t' || text[0] == L'\n' || text[0] == L'\r')
	{
		outState = StDefault;
		outConsumedChars = 1;
		return true;
	}

	// Special numbers or keywords.
	const size_t i = text.find_first_of(L" .,:;()[]{}+-*/%~=<>\t\n\r");
	const size_t ws = (i != text.npos) ? i : ln;

	const std::wstring word = text.substr(0, ws);
	if (
		word == L"true" ||
		word == L"false" ||
		word == L"nil"
	)
	{
		outState = StNumber;
		outConsumedChars = int(ws);
		return true;
	}
	else if (
		word == L"self"
	)
	{
		outState = StSelf;
		outConsumedChars = int(ws);
		return true;
	}
	else if (
		word == L"and" ||
		word == L"break" ||
		word == L"const" ||
		word == L"do" ||
		word == L"else" ||
		word == L"elseif" ||
		word == L"end" ||
		word == L"for" ||
		word == L"function" ||
		word == L"if" ||
		word == L"local" ||
		word == L"not" ||
		word == L"or" ||
		word == L"repeat" ||
		word == L"return" ||
		word == L"then" ||
		word == L"until" ||
		word == L"while"
	)
	{
		outState = StKeyword;
		outConsumedChars = int(ws);
		return true;
	}
	else if (
		word == L"assert"
	)
	{
		outState = StSpecial;
		outConsumedChars = int(ws);
		return true;
	}

	// Default as text.
	outState = StDefault;
	outConsumedChars = std::max< int >(1, int(ws));
	return true;
}

void SyntaxLanguageLua::outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const
{
	T_ASSERT(text.length() > 0);

	const size_t i = text.find_first_of(L" \t\n\r");
	if (i != text.npos)
	{
		const std::wstring word = text.substr(0, i);
		if (word == L"function")
		{
			const size_t j = text.find(L'(', i + 1);
			if (j != text.npos)
			{
				const SyntaxOutline so(SotFunction, line, text.substr(i + 1, j - i - 1));
				outOutline.push_back(so);
			}
		}
		else if (word == L"local")
		{
			const SyntaxOutline so(SotVariable, line, text.substr(i + 1));
			outOutline.push_back(so);
		}
	}
}

}
