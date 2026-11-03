/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/Canvas.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Canvas", Canvas, Widget)

void Canvas::build(html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_canvas_container");
	parent->addChild(ediv);

	createElement(L"canvas", L"wtk_canvas", ediv);
}

}
