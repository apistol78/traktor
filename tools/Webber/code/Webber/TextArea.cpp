/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/TextArea.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.TextArea", TextArea, Widget)

void TextArea::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& TextArea::getText() const
{
	return m_text;
}

void TextArea::build(html::Element* parent) const
{
	auto etextarea = createElement(L"textarea", L"wtk_textarea", parent);
	etextarea->addChild(new html::Text(m_text));
}

void TextArea::consumeValues(const std::map< int32_t, std::wstring >& values)
{
	auto it = values.find(getId());
	if (it != values.end())
		m_text = it->second;
}

}
