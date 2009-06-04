#include <sstream>
#include <stack>
#include "Xml/Element.h"
#include "Xml/Attribute.h"
#include "Xml/Text.h"
#include "Core/Heap/New.h"
#include "Core/Misc/String.h"

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
	std::wstringstream ss;
	for (Node* child = getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (is_a< Text >(child))
			ss << static_cast< Text* >(child)->getValue();
	}
	return ss.str();
}

int Element::get(const std::wstring& path, RefArray< Element >& elements)
{
	size_t i = path.find_first_of(L'/');
	if (i != path.npos)
	{
		std::wstring name = path.substr(0, i);
		std::wstring sub = path.substr(i + 1);
		
		for (Ref< Node > child = getFirstChild(); child != 0; child = child->getNextSibling())
		{
			Ref< Element > elm = dynamic_type_cast< Element* >(child);
			if (elm && elm->match(name) == true)
				elm->get(sub, elements);
		}
	}
	else
	{
		for (Ref< Node > child = getFirstChild(); child != 0; child = child->getNextSibling())
		{
			Ref< Element > elm = dynamic_type_cast< Element* >(child);
			if (elm && elm->match(path) == true)
				elements.push_back(elm);
		}
	}
	return int(elements.size());
}

Element* Element::getSingle(const std::wstring& path)
{
	RefArray< Element > elements;
	return (get(path, elements) > 0) ? elements.front() : 0;
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

		int index = 0;
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
					int n = parseString< int >(indexer.substr(0, i));
					
					// Calculate our own index.
					int nn = 0;
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
	Attribute* attr = getAttribute(name);
	if (attr == 0)
	{
		attr = gc_new< Attribute >(name);
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
	Attribute* attr;
	for (attr = m_firstAttribute; attr != 0; attr = attr->getNext())
	{
		if (attr->getName() == name)
			break;
	}
	return attr;
}

Attribute* Element::getAttribute(const std::wstring& name, const std::wstring& defaultValue) const
{
	Ref< Attribute > attr = getAttribute(name);
	if (!attr)
		attr = gc_new< Attribute >(name, defaultValue);
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

	}
}
