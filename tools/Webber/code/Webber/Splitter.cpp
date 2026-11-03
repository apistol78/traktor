/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/Splitter.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Splitter", Splitter, ILayout)

Splitter::Splitter(Direction direction)
:	m_direction(direction)
{
}

void Splitter::build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const
{
	if (children.size() < 2)
		return;

	const wchar_t* firstClass = (m_direction == Horizontal) ? L"wtk_layout_splitter_top" : L"wtk_layout_splitter_left";
	const wchar_t* secondClass = (m_direction == Horizontal) ? L"wtk_layout_splitter_bottom" : L"wtk_layout_splitter_right";

	auto ediv1 = new html::Element(L"div");
	ediv1->setAttribute(L"class", firstClass);
	children[0]->build(ediv1);
	parent->addChild(ediv1);

	auto ediv2 = new html::Element(L"div");
	ediv2->setAttribute(L"class", secondClass);
	children[1]->build(ediv2);
	parent->addChild(ediv2);
}

}
