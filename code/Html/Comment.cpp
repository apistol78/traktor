/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Io/OutputStream.h"
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

void Comment::toString(OutputStream& os) const
{
	os << L"<!-- " << m_text << L" -->" << Endl;
}

	}
}
