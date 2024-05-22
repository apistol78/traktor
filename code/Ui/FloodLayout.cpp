/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/Rect.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FloodLayout", FloodLayout, Layout)

FloodLayout::FloodLayout()
:	m_margin(0, 0)
{
}

FloodLayout::FloodLayout(const Size& margin)
:	m_margin(margin)
{
}

bool FloodLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	Ref< Widget > child = getFirstVisibleChild(widget);
	if (!child)
		return false;

	result = child->getPreferredSize(bounds) + m_margin + m_margin;
	return true;
}

void FloodLayout::update(Widget* widget)
{
	Ref< Widget > child = getFirstVisibleChild(widget);
	if (!child)
		return;

	const Rect rc = widget->getInnerRect().inflate(-m_margin.cx, -m_margin.cy);
	const WidgetRect wr = { child, rc };
	child->setChildRects(&wr, 1, false);
}

Ref< Widget > FloodLayout::getFirstVisibleChild(Widget* widget)
{
	for (Ref< Widget > child = widget->getFirstChild(); child; child = child->getNextSibling())
	{
		if (child->acceptLayout() && child->isVisible(false))
			return child;
	}
	return nullptr;
}

}
