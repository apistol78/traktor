/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Html/Attribute.h"
#include "Html/Element.h"

namespace traktor::html
{
	namespace
	{

std::wstring encodeAttributeValue(const std::wstring& value)
{
	StringOutputStream ss;
	for (auto c : value)
	{
		if (c == L'&')
			ss << L"&amp;";
		else if (c == L'\"')
			ss << L"&quot;";
		else
			ss << c;
	}
	return ss.str();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Element", Element, Node)

Element::Element(const std::wstring& name)
:	m_name(name)
{
}

std::wstring Element::getName() const
{
	return m_name;
}

std::wstring Element::getValue() const
{
	StringOutputStream ss;
	for (Ref< Node > child = getFirstChild(); child; child = child->getNextSibling())
		ss << child->getValue();
	return ss.str();
}

bool Element::hasAttribute(const std::wstring& name) const
{
	return bool(getAttribute(name) != nullptr);
}

void Element::setAttribute(const std::wstring& name, const std::wstring& value)
{
	Ref< Attribute > attr = getAttribute(name);
	if (attr == nullptr)
	{
		attr = new Attribute(name);
		attr->m_previous = nullptr;
		attr->m_next = m_firstAttribute;

		if (m_firstAttribute != nullptr)
			m_firstAttribute->m_previous = attr;

		m_firstAttribute = attr;
	}
	attr->setValue(value);
}

Attribute* Element::getFirstAttribute() const
{
	return m_firstAttribute;
}

Attribute* Element::getLastAttribute() const
{
	return m_lastAttribute;
}

Attribute* Element::getAttribute(const std::wstring& name) const
{
	Ref< Attribute > attr;
	for (attr = m_firstAttribute; attr != nullptr; attr = attr->getNext())
	{
		if (attr->getName() == name)
			break;
	}
	return attr;
}

Element* Element::getPreviousElementSibling() const
{
	Ref< Node > node = getPreviousSibling();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getPreviousSibling();
	}
	return static_cast< Element* >(node.ptr());
}

Element* Element::getNextElementSibling() const
{
	Ref< Node > node = getNextSibling();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getNextSibling();
	}
	return static_cast< Element* >(node.ptr());
}

Element* Element::getFirstElementChild() const
{
	Ref< Node > node = getFirstChild();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getNextSibling();
	}
	return static_cast< Element* >(node.ptr());
}

Element* Element::getLastElementChild() const
{
	Ref< Node > node = getLastChild();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getPreviousSibling();
	}
	return static_cast< Element* >(node.ptr());
}

void Element::innerHTML(OutputStream& os) const
{
	for (Ref< Node > child = getFirstChild(); child; child = child->getNextSibling())
		child->toString(os);
}

void Element::toString(OutputStream& os) const
{
	os << L"<" << m_name;
	for (Ref< Attribute > attribute = m_firstAttribute; attribute; attribute = attribute->getNext())
		os << L" " << attribute->getName() << L"=\"" << encodeAttributeValue(attribute->getValue()) << L"\"";

	if (getFirstChild())
	{
		os << L">";

		if (is_a< Element >(getFirstChild()))
		{
			os << IncreaseIndent;
			os << Endl;
		}

		for (Ref< Node > child = getFirstChild(); child; child = child->getNextSibling())
			child->toString(os);

		if (is_a< Element >(getFirstChild()))
			os << DecreaseIndent;

		os << L"</" << m_name << L">" << Endl;
	}
	else
	{
		// Only void elements may be self-closed in HTML.
		const wchar_t* c_voidElements[] = { L"area", L"base", L"br", L"col", L"embed", L"hr", L"img", L"input", L"link", L"meta", L"source", L"track", L"wbr" };
		bool permitClosed = false;
		for (auto voidElement : c_voidElements)
			permitClosed |= (compareIgnoreCase(voidElement, m_name) == 0);
		if (permitClosed)
			os << L"/>";
		else
			os << L"></" << m_name << L">";

		os << Endl;
	}
}

}
