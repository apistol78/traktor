/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/Float.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Float", Float, ILayout)

void Float::build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_layout_float");
	for (auto c : children)
		c->build(ediv);
	parent->addChild(ediv);
}

}
