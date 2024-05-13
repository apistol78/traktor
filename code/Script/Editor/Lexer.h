/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor::script
{

/*! Script outline parser lexer.
 * \ingroup Script
 */
class Lexer : public Object
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

	Lexer(const wchar_t* text, uint32_t length);

	int32_t next();

	double getNumber() const { return m_number; }

	const std::wstring& getString() const { return m_string; }

	const std::wstring& getWord() const { return m_word; }

	int32_t getLine() const { return m_line; }

private:
	const wchar_t* m_text;
	uint32_t m_length;
	uint32_t m_position;
	wchar_t m_peek;
	double m_number;
	std::wstring m_string;
	std::wstring m_word;
	int32_t m_line;

	wchar_t readChar();

	wchar_t peekChar();
};

}
