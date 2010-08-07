#include "Xml/Attribute.h"

namespace traktor
{
	namespace xml
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Attribute", Attribute, Object)

Attribute::Attribute(const std::wstring& name, const std::wstring& value)
:	m_name(name)
,	m_value(value)
,	m_previous(0)
{
}

const std::wstring& Attribute::getName() const
{
	return m_name;
}

void Attribute::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Attribute::getValue() const
{
	return m_value;
}

void Attribute::setValue(const std::wstring& value)
{
	m_value = value;
}

Ref< Attribute > Attribute::getPrevious() const
{
	return m_previous;
}

Ref< Attribute > Attribute::getNext() const
{
	return m_next;
}
	
	}
}
