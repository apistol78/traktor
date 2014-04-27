#ifndef traktor_script_Lexer_H
#define traktor_script_Lexer_H

#include "Core/Object.h"

namespace traktor
{
	namespace script
	{

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
}

#endif	// traktor_script_Lexer_H
