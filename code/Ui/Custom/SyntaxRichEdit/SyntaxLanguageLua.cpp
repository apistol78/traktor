#include "Core/Misc/String.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageLua.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.custom.SyntaxLanguageLua", 0, SyntaxLanguageLua, SyntaxLanguage)

std::wstring SyntaxLanguageLua::lineComment() const
{
	return L"--";
}

bool SyntaxLanguageLua::consume(const std::wstring& text, State& outState, int& outConsumedChars) const
{
	int ln = int(text.length());
	T_ASSERT (ln > 0);

	// Line comment.
	if (ln >= 2)
	{
		if (text[0] == L'-' && text[1] == L'-')
		{
			outState = StComment;
			outConsumedChars = 2;
			for (int i = 2; i < ln; ++i)
			{
				if (text[i] == L'\n' || text[i] == L'\r')
					break;
				++outConsumedChars;
			}
			return true;
		}
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
	size_t i = text.find_first_of(L" \t\n\r");
	size_t ws = (i != text.npos) ? i : ln;

	std::wstring word = text.substr(0, ws);
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
		word == L"and" ||
		word == L"break" ||
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
	outConsumedChars = 1;
	return true;
}

void SyntaxLanguageLua::outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const
{
	int ln = int(text.length());
	T_ASSERT (ln > 0);

	size_t i = text.find_first_of(L" \t\n\r");
	if (i != text.npos)
	{
		std::wstring word = text.substr(0, i);
		if (word == L"function")
		{
			size_t j = text.find(L'(', i + 1);
			if (j != text.npos)
			{
				SyntaxOutline so(SotFunction, line, text.substr(i + 1, j - i - 1));
				outOutline.push_back(so);
			}
		}
		else if (word == L"local")
		{
			SyntaxOutline so(SotVariable, line, text.substr(i + 1));
			outOutline.push_back(so);
		}
	}
}

		}
	}
}
