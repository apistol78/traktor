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

	}
}
