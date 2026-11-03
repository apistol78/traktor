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
#include "Webber/Button.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Button", Button, Widget)

Button::Button(const std::wstring& text)
:	m_text(text)
{
}

void Button::build(html::Element* parent) const
{
	auto ebutton = createElement(L"button", L"wtk_button", parent);
	ebutton->setAttribute(L"onclick", L"post_event(" + toString(getId()) + L", 1);");
	ebutton->addChild(new html::Text(m_text));
}

void Button::consume(int32_t senderId, int32_t action)
{
	if (senderId == getId())
		m_clicked.raise();
}

Event& Button::clicked()
{
	return m_clicked;
}

}
