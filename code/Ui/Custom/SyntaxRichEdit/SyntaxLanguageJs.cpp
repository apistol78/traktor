#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageJs.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.custom.SyntaxLanguageJs", 0, SyntaxLanguageJs, SyntaxLanguage)

std::wstring SyntaxLanguageJs::lineComment() const
{
	return L"//";
}

bool SyntaxLanguageJs::consume(const std::wstring& text, State& outState, int& outConsumedChars) const
{
	int ln = int(text.length());
	T_ASSERT (ln > 0);

	// Line comment.
	if (ln >= 2)
	{
		if (text[0] == L'/' && text[1] == L'/')
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
		word == L"break" ||
		word == L"else" ||
		word == L"for" ||
		word == L"function" ||
		word == L"if" ||
		word == L"return" ||
		word == L"var" ||
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

void SyntaxLanguageJs::outline(int32_t line, const std::wstring& text, std::list< SyntaxOutline >& outOutline) const
{
}

		}
	}
}
