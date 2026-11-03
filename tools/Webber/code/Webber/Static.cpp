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
#include "Webber/Static.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Static", Static, Widget)

Static::Static(const std::wstring& text)
:	m_text(text)
{
}

void Static::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Static::getText() const
{
	return m_text;
}

void Static::build(html::Element* parent) const
{
	auto ediv = createElement(L"div", L"wtk_static", parent);
	ediv->addChild(new html::Text(m_text));
}

}
