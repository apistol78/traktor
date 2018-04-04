/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <numeric>
#include <functional>
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Rect.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace ui
	{

namespace
{

void parseDefinition(const std::wstring& def, std::vector< int >& out)
{
	std::vector< std::wstring > tmp;
	
	if (!Split< std::wstring >::any(def, L",;", tmp))
		return;
	
	for (std::vector< std::wstring >::iterator i = tmp.begin(); i != tmp.end(); ++i)
	{
		std::wstring str = *i;
		if (str == L"*")
			out.push_back(0);
		else
		{
			int n = abs(atoi(wstombs(str).c_str()));
			if (*(str.end() - 1) == '%')
				out.push_back(-n);
			else
				out.push_back(n);
		}
	}
}

void calculate(
	const Size& avail,
	const std::vector< int >& cdef,
	const std::vector< int >& rdef,
	const std::vector< Widget* >& children,
	std::vector< int >& w,
	std::vector< int >& h
)
{
	int nc = int(cdef.size());
	int nr = int((children.size() + nc - 1) / std::max(nc, 1));

	w.resize(nc);
	for (int c = 0; c < nc; ++c)
	{
		if (cdef[c] == 0)
		{
			// Find widest child in this column.
			w[c] = 0;
			for (int r = 0; r < nr; ++r)
			{
				int i = c + r * nc;
				if (i < int(children.size()))
					w[c] = std::max< int >(w[c], children[i]->getPreferedSize().cx);
			}
		}
		else
		{
			// Absolute or relative width.
			w[c] = cdef[c];
		}
	}
	
	// Calculate occupied width by either child prefered size or absolute size.
	int wt = 0;
	int wrt = 0;
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
	for (int r = 0; r < nr; ++r)
	{
		if (rdef[r % rdef.size()] == 0)
		{
			// Find highest child in this row.
			h[r] = 0;
			for (int c = 0; c < nc; ++c)
			{
				int i = c + r * nc;
				if (i < int(children.size()))
					h[r] = std::max< int >(h[r], children[i]->getPreferedSize().cy);
			}
		}
		else
		{
			// Absolute or relative width.
			h[r] = rdef[r % rdef.size()];
		}
	}
	
	// Calculate occupied height by either child prefered size or absolute size.
	int ht = 0;
	int hrt = 0;
	for (int r = 0; r < nr; ++r)
	{
		if (h[r] > 0)
			ht += h[r];
		else
			hrt += -h[r];
	}
	
	// Fix relative heights.
	ht = std::max(avail.cy - ht, 0);
	for (int r = 0; r < nr; ++r)
	{
		if (h[r] < 0)
			h[r] = (ht * -h[r]) / hrt;
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TableLayout", TableLayout, Layout)

TableLayout::TableLayout(const std::wstring& cdef, const std::wstring& rdef, int margin, int pad)
:	m_margin(margin, margin)
,	m_pad(pad, pad)
{
	parseDefinition(cdef, m_cdef);
	parseDefinition(rdef, m_rdef);
}

bool TableLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	std::vector< Widget* > children;
	for (Widget* child = widget->getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (child->acceptLayout())
			children.push_back(child);
	}
	if (children.size() <= 0)
		return false;

	int nc = int(m_cdef.size());
	int nr = int((children.size() + nc - 1) / std::max(nc, 1));
	
	std::vector< int > w;
	std::vector< int > h;
	calculate(bounds, m_cdef, m_rdef, children, w, h);

	result.cx = std::accumulate(w.begin(), w.end(), m_margin.cx * 2 + m_pad.cx * (nc - 1));
	result.cy = std::accumulate(h.begin(), h.end(), m_margin.cy * 2 + m_pad.cy * (nr - 1));
	return true;
}

void TableLayout::update(Widget* widget)
{
	Rect inner = widget->getInnerRect();

	std::vector< Widget* > children;
	for (Widget* child = widget->getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (child->acceptLayout())
			children.push_back(child);
	}
	if (children.size() <= 0)
		return;

	int nc = int(m_cdef.size());
	int nr = int((children.size() + nc - 1) / std::max(nc, 1));

	Size avail = inner.getSize() - m_margin - m_margin - Size(m_pad.cx * (nc - 1), m_pad.cy * (nr - 1));

	std::vector< int > w;
	std::vector< int > h;
	calculate(avail, m_cdef, m_rdef, children, w, h);

	std::vector< WidgetRect > rects(children.size());

	Point tl = inner.getTopLeft() + m_margin;
	for (int32_t i = 0; i < int32_t(children.size()); ++i)
	{
		int32_t c = i % std::max(nc, 1);
		int32_t r = i / std::max(nc, 1);
		
		Size pf = children[i]->getPreferedSize();
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
			tl.x += w[c] + m_pad.cx;
		}
		else
		{
			tl.x = inner.left + m_margin.cx;
			tl.y += h[r] + m_pad.cy;
		}
	}

	widget->setChildRects(rects);
}

	}
}
