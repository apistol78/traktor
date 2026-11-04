/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/Edit.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Edit", Edit, Widget)

void Edit::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Edit::getText() const
{
	return m_text;
}

void Edit::setPassword(bool password)
{
	m_password = password;
}

void Edit::build(html::Element* parent) const
{
	auto einput = createElement(L"input", L"wtk_edit", parent);
	if (m_password)
		einput->setAttribute(L"type", L"password");
	else
	{
		einput->setAttribute(L"type", L"text");
		einput->setAttribute(L"value", m_text);
	}
}

void Edit::consumeValues(const std::map< int32_t, std::wstring >& values)
{
	auto it = values.find(getId());
	if (it != values.end())
		m_text = it->second;
}

}
