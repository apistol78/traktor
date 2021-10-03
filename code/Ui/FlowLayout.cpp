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

FlowLayout::FlowLayout(int32_t marginX, int32_t marginY, int32_t padX, int32_t padY)
:	m_margin(marginX, marginY)
,	m_pad(padX, padY)
{
}

bool FlowLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	std::vector< WidgetRect > rects;
	if (!calculateRects(widget, rects))
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
	if (!calculateRects(widget, widgetRects))
		return;
	widget->setChildRects(&widgetRects[0], (uint32_t)widgetRects.size());
}

bool FlowLayout::calculateRects(Widget* widget, std::vector< WidgetRect >& outRects) const
{
	Point pos(m_margin.cx, m_margin.cy);
	int32_t max = 0;

	Rect innerRect = widget->getInnerRect();
	if (innerRect.area() <= 0)
		return false;

	for (Widget* child = widget->getFirstChild(); child != nullptr; child = child->getNextSibling())
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

		ext.x = std::min< int32_t >(ext.x, innerRect.right - m_margin.cx);
		outRects.push_back(WidgetRect(child, Rect(pos, ext)));

		pos.x = ext.x + m_pad.cx;
	}

	return true;
}

	}
}
