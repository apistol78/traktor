/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Html/Attribute.h"
#include "Html/Element.h"

namespace traktor
{
	namespace html
	{

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
	return bool(getAttribute(name) != 0);
}

void Element::setAttribute(const std::wstring& name, const std::wstring& value)
{
	Ref< Attribute > attr = getAttribute(name);
	if (attr == 0)
	{
		attr = new Attribute(name);
		attr->m_previous = 0;
		attr->m_next = m_firstAttribute;
		
		if (m_firstAttribute != 0)
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
	for (attr = m_firstAttribute; attr != 0; attr = attr->getNext())
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
		os << L" " << attribute->getName() << L"=\"" << attribute->getValue() << L"\"";

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
		bool permitClosed = !bool(
			compareIgnoreCase< std::wstring >(L"script", m_name) == 0 ||
			compareIgnoreCase< std::wstring >(L"link", m_name) == 0
		);
		if (permitClosed)
			os << L"/>";
		else
			os << L"></" << m_name << L">";

		os << Endl;
	}
}

	}
}
