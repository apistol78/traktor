#include <sstream>
#include <stack>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Xml/Attribute.h"
#include "Xml/Element.h"
#include "Xml/Text.h"

namespace traktor
{
	namespace xml
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Element", Element, Node)

Element::Element(const std::wstring& name)
:	m_name(name)
{
}

std::wstring Element::getName() const
{
	return m_name;
}

void Element::setName(const std::wstring& name)
{
	m_name = name;
}

std::wstring Element::getValue() const
{
	StringOutputStream ss;
	for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (is_a< Text >(child))
			ss << static_cast< Text* >(child)->getValue();
	}
	return ss.str();
}

void Element::setValue(const std::wstring& value)
{
	Ref< Node > child = getFirstChild();
	while (child)
	{
		Ref< Node > next = child->getNextSibling();
		if (is_a< Text >(child))
			removeChild(child);
		child = next;
	}

	if (!value.empty())
		insertBefore(new Text(value), 0);
}

void Element::write(OutputStream& os) const
{
	os << L"<" << m_name;
	for (Attribute* attr = getFirstAttribute(); attr; attr = attr->getNext())
		os << L" " << attr->getName() << L"=\"" << attr->getValue() << L"\"";

	if (getFirstChild())
	{
		os << L">";
		os << IncreaseIndent;

		uint32_t childElements = 0, childText = 0;
		for (Node* child = getFirstChild(); child; child = child->getNextSibling())
		{
			if (is_a< Element >(child))
				++childElements;
			else if (is_a< Text >(child))
				++childText;
		}

		if (childElements > 0 || childText > 1)
			os << Endl;

		Node::write(os);

		os << DecreaseIndent;
		os << L"</" << m_name << L">" << Endl;
	}
	else
		os << L"/>" << Endl;
}

int32_t Element::get(const std::wstring& path, RefArray< Element >& outElements) const
{
	size_t i = path.find_first_of(L'/');
	if (i != path.npos)
	{
		std::wstring name = path.substr(0, i);
		std::wstring sub = path.substr(i + 1);
		
		for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
		{
			Element* elm = dynamic_type_cast< Element* >(child);
			if (elm && elm->match(name) == true)
				elm->get(sub, outElements);
		}
	}
	else
	{
		for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
		{
			Element* elm = dynamic_type_cast< Element* >(child);
			if (elm && elm->match(path) == true)
				outElements.push_back(elm);
		}
	}
	return int32_t(outElements.size());
}

Element* Element::getSingle(const std::wstring& path) const
{
	size_t i = path.find_first_of(L'/');
	if (i != path.npos)
	{
		std::wstring name = path.substr(0, i);
		std::wstring sub = path.substr(i + 1);
		
		for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
		{
			Element* elm = dynamic_type_cast< Element* >(child);
			if (elm && elm->match(name) == true)
			{
				Element* found = elm->getSingle(sub);
				if (found)
					return found;
			}
		}
	}
	else
	{
		for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
		{
			Element* elm = dynamic_type_cast< Element* >(child);
			if (elm && elm->match(path) == true)
				return elm;
		}
	}

	return 0;
}

std::wstring Element::getPath() const
{
	std::stack< const Element* > elm;

	for (const Element* e = this; e != 0; e = static_cast< const Element* >(e->getParent()))
		elm.push(e);

	std::wstringstream ss;
	while (!elm.empty())
	{
		const Element* e = elm.top();

		int32_t index = 0;
		for (Node* node = e->getPreviousSibling(); node != 0; node = node->getPreviousSibling())
		{
			if (is_a< Element >(node))
			{
				if (e->getName() == node->getName())
					++index;
			}
		}

		if (index)
			ss << L"/" << elm.top()->getName() << L"[" << index << L"]";
		else
			ss << L"/" << elm.top()->getName();

		elm.pop();
	}

	return ss.str();
}

bool Element::match(const std::wstring& condition)
{
	size_t i = condition.find_first_of(L'[');
	if (i != condition.npos && *(condition.end() - 1) == L']')
	{
		std::wstring name = condition.substr(0, i);
		if (name != L"*" && name != getName())
			return false;

		std::wstring indexer = condition.substr(i + 1, condition.length() - i - 2);
		if (indexer[0] == L'@')
		{
			i = indexer.find_first_of(L'=');
			if (i == indexer.npos)
				return false;
				
			std::wstring attribName = indexer.substr(1, i - 1);
			Attribute* attrib = getAttribute(attribName);
			if (attrib == 0)
				return false;
				
			std::wstring attribValue = indexer.substr(i + 1);
			if (attrib->getValue() != attribValue)
				return false;
		}
		else
		{
			i = indexer.find_first_of(L'=');
			if (i == indexer.npos)
			{
				// Check last() operator.
				if (indexer == L"last()")
				{
					// Am I the last element?
					for (Node* node = getNextSibling(); node != 0; node = node->getNextSibling())
					{
						if (is_a< Element >(node))
						{
							if (name == L"*" || name == node->getName())
								return false;
						}
					}
				}
				else
				{
					int32_t n = parseString< int32_t >(indexer);
					
					// Calculate our own index.
					int32_t nn = 0;
					for (Node* node = getPreviousSibling(); node != 0; node = node->getPreviousSibling())
					{
						if (is_a< Element >(node))
						{
							if (name == L"*" || name == node->getName())
								nn++;
						}
					}
					
					// Does indices match?
					if (n != nn)
						return false;
				}
			}
			else
			{
				std::wstring childName = indexer.substr(0, i);
				Element* elm = getChildElementByName(childName);
				if (elm == 0)
					return false;
				
				std::wstring childValue = indexer.substr(i + 1);
				if (elm->getValue() != childValue)
					return false;
			}
		}
	}
	else
	{
		if (condition != L"*" && condition != getName())
			return false;
	}
	return true;
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
		attr->m_previous = m_lastAttribute;
		attr->m_next = 0;
		
		if (!m_firstAttribute)
			m_firstAttribute = attr;

		if (m_lastAttribute)
			m_lastAttribute->m_next = attr;

		m_lastAttribute = attr;
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
	Attribute* attr;
	for (attr = m_firstAttribute; attr != 0; attr = attr->getNext())
	{
		if (attr->getName() == name)
			break;
	}
	return attr;
}

Ref< Attribute > Element::getAttribute(const std::wstring& name, const std::wstring& defaultValue) const
{
	Ref< Attribute > attr = getAttribute(name);
	if (!attr)
		attr = new Attribute(name, defaultValue);
	return attr;
}

Element* Element::getChildElementByName(const std::wstring& name)
{
	for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (is_a< Element >(child))
		{
			Element* elm = static_cast< Element* >(child);
			if (elm->getName() == name)
				return elm;
		}
	}
	return 0;
}

Ref< Element > Element::clone() const
{
	Ref< Element > elm = new Element(m_name);
	
	for (const Attribute* attr = m_firstAttribute; attr; attr = attr->getNext())
		elm->setAttribute(attr->getName(), attr->getValue());
	
	cloneChildren(elm);

	return elm;
}

Ref< Node > Element::cloneUntyped() const
{
	return clone();
}

	}
}
