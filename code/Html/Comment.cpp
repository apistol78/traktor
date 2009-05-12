#include <sstream>
#include "Html/Comment.h"
#include "Core/Io/Stream.h"

namespace traktor
{
	namespace html
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Comment", Comment, Node)

Comment::Comment(const std::wstring& text)
:	m_text(text)
{
}

std::wstring Comment::getName() const
{
	return L"#COMMENT#";
}

std::wstring Comment::getValue() const
{
	return m_text;
}

void Comment::writeHtml(Stream* stream)
{
	std::wstringstream ss;
	ss << L"<!-- " << m_text << L" -->" << std::endl;
	std::wstring tmp = ss.str();
	stream->write(tmp.c_str(), int(tmp.length()));
}

	}
}
