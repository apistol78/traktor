/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Html/Comment.h"

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

void Comment::writeHtml(IStream* stream)
{
	StringOutputStream ss;
	ss << L"<!-- " << m_text << L" -->" << Endl;
	std::wstring tmp = ss.str();
	stream->write(tmp.c_str(), int(tmp.length()));
}

	}
}
