/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_Lexer_H
#define ddc_Lexer_H

#include <Core/Object.h>
#include <Core/Io/StringReader.h>
#include "Ddc/Parser.h"

namespace ddc
{

class Lexer : public traktor::Object
{
	T_RTTI_CLASS;

public:
	enum LexerType
	{
		LtEndOfFile = 0,
		LtNumber = -1,
		LtString = -2,
		LtWord = -3
	};

	Lexer(traktor::IStream* stream);

	int32_t next();

	double getNumber() const { return m_number; }

	const std::wstring& getString() const { return m_string; }

	const std::wstring& getWord() const { return m_word; }

private:
	traktor::StringReader m_reader;
	wchar_t m_peek;
	double m_number;
	std::wstring m_string;
	std::wstring m_word;

	wchar_t readChar();

	wchar_t peekChar();
};

}

#endif	// ddc_Lexer_H
