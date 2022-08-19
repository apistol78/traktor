#include "Ui/CenterLayout.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CenterLayout", CenterLayout, Layout)

CenterLayout::CenterLayout(uint32_t centerAxis)
:	m_centerAxis(centerAxis)
{
}

bool CenterLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	result = widget->getPreferredSize(bounds);
	return true;
}

void CenterLayout::update(Widget* widget)
{
	Widget* child = widget->getFirstChild();
	if (child)
	{
		const Rect rc = widget->getInnerRect();
		Size sz = child->getPreferredSize(rc.getSize());

		const float ratio = float(sz.cx) / sz.cy;
		if (sz.cx > rc.getWidth())
		{
			sz.cx = rc.getWidth();
			sz.cy = (int32_t)(sz.cx / ratio);
		}
		if (sz.cy > rc.getHeight())
		{
			sz.cy = rc.getHeight();
			sz.cx = (int32_t)(sz.cy * ratio);
		}

		const int32_t x = (m_centerAxis & CaHorizontal) != 0 ? (rc.getWidth() - sz.cx) / 2 : 0;
		const int32_t y = (m_centerAxis & CaVertical) != 0 ? (rc.getHeight() - sz.cy) / 2 : 0;

		child->setRect(Rect(x, y, x + sz.cx, y + sz.cy));
	}
}

	}
}
