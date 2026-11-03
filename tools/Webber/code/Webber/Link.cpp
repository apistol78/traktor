/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/Link.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Link", Link, Widget)

Link::Link(const std::wstring& text)
:	m_text(text)
{
}

void Link::build(html::Element* parent) const
{
	auto ea = createElement(L"a", L"wtk_link", parent);
	ea->setAttribute(L"href", L"#");
	ea->setAttribute(L"onclick", L"post_event(" + toString(getId()) + L", 1); return false;");
	ea->addChild(new html::Text(m_text));
}

void Link::consume(int32_t senderId, int32_t action)
{
	if (senderId == getId())
		m_clicked.raise();
}

Event& Link::clicked()
{
	return m_clicked;
}

}
