/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Misc/String.h>
#include "Ddc/Lexer.h"

using namespace traktor;

namespace ddc
{
	namespace
	{

enum CharacterType
{
	CtUnknown = 0,
	CtWhitespace = 1,
	CtAlpha = 2,
	CtDigit = 4,
	CtQuote = 8
};

int32_t characterType(wchar_t ch)
{
	if (ch >= L'a' && ch <= L'z')
		return CtAlpha;
	else if (ch >= L'A' && ch <= L'Z')
		return CtAlpha;
	else if (ch >= L'0' && ch <= L'9' || ch == L'-')
		return CtDigit;
	else if (ch == L'\"')
		return CtQuote;
	else if(ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r')
		return CtWhitespace;
	else
		return CtUnknown;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"ddc.Lexer", Lexer, Object)

Lexer::Lexer(IStream* stream)
:	m_reader(stream, new Utf8Encoding())
,	m_peek(0)
,	m_number(0.0)
{
}

int32_t Lexer::next()
{
	wchar_t ch = readChar();
	int32_t chtype = characterType(ch);

	// Strip whitespace.
	while ((chtype & CtWhitespace) == CtWhitespace)
	{
		if (!(ch = readChar()))
			return LtEndOfFile;
		chtype = characterType(ch);
	}

	// Parse number.
	if ((chtype & CtDigit) == CtDigit)
	{
		StringOutputStream ss;
		while ((chtype & CtDigit) == CtDigit)
		{
			ss << ch;
			ch = peekChar();
			chtype = characterType(ch);
			if ((chtype & CtDigit) == CtDigit)
				readChar();
		}
		m_number = parseString< double >(ss.str());
		return LtNumber;
	}

	// Parse word.
	if ((chtype & CtAlpha) == CtAlpha)
	{
		StringOutputStream ss;
		while ((chtype & (CtAlpha | CtDigit)) != 0)
		{
			ss << ch;
			ch = peekChar();
			chtype = characterType(ch);
			if ((chtype & (CtAlpha | CtDigit)) != 0)
				readChar();
		}
		m_word = ss.str();
		return LtWord;
	}

	// Parse quoted string.
	if ((chtype & CtQuote) == CtQuote)
	{
		StringOutputStream ss;
		for (;;)
		{
			wchar_t ch2 = readChar();
			if (ch2 == ch)
				break;
			if (ch2 == L'\\')
			{
				ch2 = readChar();
				switch (ch2)
				{
				case L'n':
					ch2 = L'\n';
					break;
				case L'r':
					ch2 = L'\r';
					break;
				case L't':
					ch2 = L'\t';
					break;
				}
			}
			ss << ch2;
		}
		m_string = ss.str();
		return LtString;
	}

	return ch;
}

wchar_t Lexer::readChar()
{
	wchar_t ch;
	if (m_peek != 0)
	{
		ch = m_peek;
		m_peek = 0;
	}
	else
		ch = m_reader.readChar();
	return ch;
}

wchar_t Lexer::peekChar()
{
	if (m_peek == 0)
		m_peek = readChar();
	return m_peek;
}

}
