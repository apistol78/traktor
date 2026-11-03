/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Html/Text.h"

namespace traktor::html
{
	namespace
	{

std::wstring encodeCharacterEntities(const std::wstring& text)
{
	StringOutputStream ss;
	for (auto c : text)
	{
		switch (c)
		{
		case L'&':
			ss << L"&amp;";
			break;
		case L'<':
			ss << L"&lt;";
			break;
		case L'>':
			ss << L"&gt;";
			break;
		// Line breaks as references; output stream indentation would otherwise alter text.
		case L'\n':
			ss << L"&#10;";
			break;
		case L'\r':
			ss << L"&#13;";
			break;
		default:
			ss << c;
			break;
		}
	}
	return ss.str();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Text", Text, Node)

Text::Text(const std::wstring& text)
:	m_text(text)
{
}

std::wstring Text::getName() const
{
	return L"#TEXT#";
}

std::wstring Text::getValue() const
{
	return m_text;
}

void Text::toString(OutputStream& os) const
{
	os << encodeCharacterEntities(m_text);
}

}
