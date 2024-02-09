/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/FlowLayout.h"
#include "Ui/Container.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FlowLayout", FlowLayout, Layout)

FlowLayout::FlowLayout(Unit margin, Unit pad)
:	m_margin(margin.get(), margin.get())
,	m_pad(pad.get(), pad.get())
{
}

FlowLayout::FlowLayout(Unit marginX, Unit marginY, Unit padX, Unit padY)
:	m_margin(marginX.get(), marginY.get())
,	m_pad(padX.get(), padY.get())
{
}

bool FlowLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	std::vector< WidgetRect > rects;
	if (!calculateRects(widget, bounds, rects))
		return false;

	result.cx =
	result.cy = 0;
	for (const auto& wr : rects)
	{
		result.cx = std::max< int >(result.cx, wr.rect.right);
		result.cy = std::max< int >(result.cy, wr.rect.bottom);
	}

	result += m_margin;
	return true;
}

void FlowLayout::update(Widget* widget)
{
	std::vector< WidgetRect > widgetRects;
	if (!calculateRects(widget, widget->getInnerRect().getSize(), widgetRects))
		return;
	widget->setChildRects(&widgetRects[0], (uint32_t)widgetRects.size(), true);
}

bool FlowLayout::calculateRects(Widget* widget, const Size& bounds, std::vector< WidgetRect >& outRects) const
{
	Point pos(m_margin.cx, m_margin.cy);
	int32_t max = 0;

	if (bounds.cx * bounds.cy <= 0)
		return false;

	for (Widget* child = widget->getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		if (!child->acceptLayout())
			continue;

		const Size pref = child->getPreferredSize(bounds);
		Point ext = pos + pref;

		if (ext.x > bounds.cx - m_margin.cx && max > 0)
		{
			pos = Point(m_margin.cx, max + m_pad.cy);
			ext = pos + pref;
			max = 0;
		}

		if (ext.y > max)
			max = ext.y;

		ext.x = std::min< int32_t >(ext.x, bounds.cx - m_margin.cx);
		outRects.push_back(WidgetRect(child, Rect(pos, ext)));

		pos.x = ext.x + m_pad.cx;
	}

	return true;
}

}
