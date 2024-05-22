/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <numeric>
#include <functional>
#include "Core/Containers/StaticVector.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Rect.h"

namespace traktor::ui
{
	namespace
	{

typedef StaticVector< Widget*, 32 * 32 > childrenVector_t;
typedef StaticVector< int32_t, 32 > dimensionVector_t;

void parseDefinition(const std::wstring& def, AlignedVector< int32_t >& out)
{
	out.reserve(32);
	for (auto str : StringSplit< std::wstring >(def,  L",;"))
	{
		if (str == L"*")
			out.push_back(0);
		else
		{
			const int32_t n = abs(atoi(wstombs(str).c_str()));
			if (*(str.end() - 1) == '%')
				out.push_back(-n);
			else
				out.push_back(n);
		}
	}
}

void calculate(
	const Size& avail,
	const AlignedVector< int32_t >& cdef,
	const AlignedVector< int32_t >& rdef,
	const childrenVector_t& children,
	dimensionVector_t& w,
	dimensionVector_t& h
)
{
	const int32_t nc = (int32_t)cdef.size();
	const int32_t nr = (int32_t)((children.size() + nc - 1) / std::max(nc, 1));

	w.resize(nc);
	for (int32_t c = 0; c < nc; ++c)
	{
		if (cdef[c] == 0)
		{
			// Find widest child in this column.
			w[c] = 0;
			for (int32_t r = 0; r < nr; ++r)
			{
				int32_t i = c + r * nc;
				if (i < (int32_t)children.size())
					w[c] = std::max< int32_t >(w[c], children[i]->getPreferredSize(avail).cx);
			}
		}
		else
		{
			// Absolute or relative width.
			w[c] = cdef[c];
		}
	}

	// Calculate occupied width by either child preferred size or absolute size.
	int32_t wt = 0;
	int32_t wrt = 0;
	for (int c = 0; c < nc; ++c)
	{
		if (w[c] > 0)
			wt += w[c];
		else
			wrt += -w[c];
	}

	// Fix relative widths.
	wt = std::max(avail.cx - wt, 0);
	for (int c = 0; c < nc; ++c)
	{
		if (w[c] < 0)
			w[c] = (wt * -w[c]) / wrt;
	}

	h.resize(nr);
	for (int32_t r = 0; r < nr; ++r)
	{
		if (rdef[r % rdef.size()] == 0)
		{
			// Find highest child in this row.
			h[r] = 0;
			for (int32_t c = 0; c < nc; ++c)
			{
				int32_t i = c + r * nc;
				if (i < (int32_t)children.size())
					h[r] = std::max< int32_t >(h[r], children[i]->getPreferredSize(avail).cy);
			}
		}
		else
		{
			// Absolute or relative width.
			h[r] = rdef[r % rdef.size()];
		}
	}

	// Calculate occupied height by either child prefered size or absolute size.
	int32_t ht = 0;
	int32_t hrt = 0;
	for (int32_t r = 0; r < nr; ++r)
	{
		if (h[r] > 0)
			ht += h[r];
		else
			hrt += -h[r];
	}

	// Fix relative heights.
	ht = std::max(avail.cy - ht, 0);
	for (int32_t r = 0; r < nr; ++r)
	{
		if (h[r] < 0)
			h[r] = (ht * -h[r]) / hrt;
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TableLayout", TableLayout, Layout)

TableLayout::TableLayout(const std::wstring& cdef, const std::wstring& rdef, Unit margin, Unit pad)
:	m_margin(margin)
,	m_pad(pad)
{
	parseDefinition(cdef, m_cdef);
	parseDefinition(rdef, m_rdef);
}

bool TableLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	const Rect inner = widget->getInnerRect();
	const Size margin = { widget->pixel(m_margin), widget->pixel(m_margin) };
	const Size pad = { widget->pixel(m_pad), widget->pixel(m_pad) };

	result = Size(0, 0);

	childrenVector_t children;
	for (Widget* child = widget->getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		if (child->acceptLayout())
			children.push_back(child);
	}
	if (children.size() <= 0)
		return false;

	int32_t nc = (int32_t)m_cdef.size();

	dimensionVector_t w;
	dimensionVector_t h;
	calculate(bounds, m_cdef, m_rdef, children, w, h);

	Point tl = inner.getTopLeft() + margin;
	for (int32_t i = 0; i < (int32_t)children.size(); ++i)
	{
		int32_t c = i % std::max(nc, 1);
		int32_t r = i / std::max(nc, 1);

		Size pf = children[i]->getPreferredSize(bounds);
		Size sz(
			std::min< int32_t >(w[c], pf.cx),
			std::min< int32_t >(h[r], pf.cy)
		);

		result.cx = std::max< int32_t >(result.cx, tl.x + sz.cx);
		result.cy = std::max< int32_t >(result.cy, tl.y + sz.cy);

		if (c < nc - 1)
		{
			tl.x += w[c] + pad.cx;
		}
		else
		{
			tl.x = inner.left + margin.cx;
			tl.y += h[r] + pad.cy;
		}
	}

	result.cx += margin.cx;
	result.cy += margin.cy;
	return true;
}

void TableLayout::update(Widget* widget)
{
	const Rect inner = widget->getInnerRect();
	const Size margin = { widget->pixel(m_margin), widget->pixel(m_margin) };
	const Size pad = { widget->pixel(m_pad), widget->pixel(m_pad) };

	childrenVector_t children;
	for (Widget* child = widget->getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		if (child->acceptLayout())
			children.push_back(child);
	}
	if (children.size() <= 0)
		return;

	int32_t nc = (int32_t)m_cdef.size();
	int32_t nr = (int32_t)((children.size() + nc - 1) / std::max(nc, 1));

	Size avail = inner.getSize() - margin - margin - Size(pad.cx * (nc - 1), pad.cy * (nr - 1));

	dimensionVector_t w;
	dimensionVector_t h;
	calculate(avail, m_cdef, m_rdef, children, w, h);

	StaticVector< WidgetRect, 32 > rects(children.size());

	Point tl = inner.getTopLeft() + margin;
	for (int32_t i = 0; i < (int32_t)children.size(); ++i)
	{
		int32_t c = i % std::max(nc, 1);
		int32_t r = i / std::max(nc, 1);

		Size pf = children[i]->getPreferredSize(avail);
		Size mx = children[i]->getMaximumSize();

		Size sz(
			std::min< int32_t >(w[c], mx.cx),
			std::min< int32_t >(h[r], mx.cy)
		);

		Point ctl = tl;

		switch (children[i]->getHorizontalAlign())
		{
		case AnCenter:
			ctl.x = tl.x + std::max< int32_t >((sz.cx - pf.cx) / 2, 0);
			break;

		case AnRight:
			ctl.x = tl.x + std::max< int32_t >(sz.cx - pf.cx, 0);
			break;

		default:
			break;
		}

		switch (children[i]->getVerticalAlign())
		{
		case AnCenter:
			ctl.y = tl.y + std::max< int32_t >((sz.cy - pf.cy) / 2, 0);
			break;

		case AnBottom:
			ctl.y = tl.y + std::max< int32_t >(sz.cy - pf.cy, 0);
			break;

		default:
			break;
		}

		rects[i].widget = children[i];
		rects[i].rect = Rect(ctl, sz);

		if (c < nc - 1)
		{
			tl.x += w[c] + pad.cx;
		}
		else
		{
			tl.x = inner.left + margin.cx;
			tl.y += h[r] + pad.cy;
		}
	}

	widget->setChildRects(rects.c_ptr(), (uint32_t)rects.size(), false);
}

}
