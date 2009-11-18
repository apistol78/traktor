#include "Html/Text.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace html
	{

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

void Text::writeHtml(IStream* stream)
{
	stream->write(m_text.c_str(), int(m_text.length()));
	Node::writeHtml(stream);
}

	}
}
