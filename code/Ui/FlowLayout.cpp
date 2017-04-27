/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/FlowLayout.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FlowLayout", FlowLayout, Layout)

FlowLayout::FlowLayout()
:	m_margin(4, 4)
,	m_pad(4, 4)
{
}

FlowLayout::FlowLayout(int marginX, int marginY, int padX, int padY) :
	m_margin(marginX, marginY),
	m_pad(padX, padY)
{
}

bool FlowLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	std::vector< WidgetRect > rects;
	
	calculateRects(widget, rects);
	
	result.cx = result.cy = 0;
	for (std::vector< WidgetRect >::iterator i = rects.begin(); i != rects.end(); ++i)
	{
		result.cx = std::max< int >(result.cx, i->rect.right);
		result.cy = std::max< int >(result.cy, i->rect.bottom);
	}
	
	result += m_margin;
	
	return true;
}

void FlowLayout::update(Widget* widget)
{
	std::vector< WidgetRect > widgetRects;
	calculateRects(widget, widgetRects);
	widget->setChildRects(widgetRects);
}

void FlowLayout::calculateRects(Widget* widget, std::vector< WidgetRect >& outRects)
{
	Point pos(m_margin.cx, m_margin.cy);
	int max = 0;
	
	Rect innerRect = widget->getInnerRect();
	for (Widget* child = widget->getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (!child->acceptLayout())
			continue;

		Size pref = child->getPreferedSize();
		Point ext = pos + pref;
		
		if (ext.x > innerRect.right - m_margin.cx && max > 0)
		{
			pos = Point(m_margin.cx, max + m_pad.cy);
			ext = pos + pref;
			max = 0;
		}
		
		if (ext.y > max)
			max = ext.y;
		
		ext.x = std::min< int >(ext.x, innerRect.right - m_margin.cx);
		outRects.push_back(WidgetRect(child, Rect(pos, ext)));
		
		pos.x = ext.x + m_pad.cx;
	}
}

	}
}
