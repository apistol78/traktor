/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/OutputStream.h"
#include "Xml/Text.h"

namespace traktor
{
	namespace xml
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
}
