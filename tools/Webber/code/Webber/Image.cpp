/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/Image.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Image", Image, Widget)

Image::Image(const std::wstring& source, const std::wstring& text)
:	m_source(source)
,	m_text(text)
{
}

void Image::build(html::Element* parent) const
{
	auto eimg = createElement(L"img", L"wtk_image", parent);
	eimg->setAttribute(L"src", m_source);
	eimg->setAttribute(L"alt", m_text);
}

}
