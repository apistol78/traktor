#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageLua.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.ui.custom.SyntaxLanguageLua", SyntaxLanguageLua, SyntaxLanguage)

void SyntaxLanguageLua::begin()
{
}

bool SyntaxLanguageLua::consume(const std::wstring& text, State& outState, int& outConsumedChars)
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
		for (; i < ln && (text[i] >= L'0' && text[i] <= L'9') || text[i] == L'.'; ++i)
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

	// Default as text.
	outState = StDefault;
	outConsumedChars = 1;
	return true;
}

void SyntaxLanguageLua::newLine()
{
}

		}
	}
}
