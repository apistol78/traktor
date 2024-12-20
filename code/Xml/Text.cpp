/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStream.h"
#include "Xml/Text.h"

namespace traktor::xml
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Text", Text, Node)

Text::Text(const std::wstring& text)
:	m_text(text)
{
}

std::wstring Text::getValue() const
{
	return m_text;
}

void Text::setValue(const std::wstring& value)
{
	m_text = value;
}

void Text::write(OutputStream& os) const
{
	os << m_text;
}

Ref< Text > Text::clone() const
{
	Ref< Text > textClone = new Text(m_text);
	cloneChildren(textClone);
	return textClone;
}

Ref< Node > Text::cloneUntyped() const
{
	return clone();
}

}
