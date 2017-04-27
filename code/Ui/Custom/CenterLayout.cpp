/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/CenterLayout.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.CenterLayout", CenterLayout, Layout)

CenterLayout::CenterLayout(uint32_t centerAxis)
:	m_centerAxis(centerAxis)
{
}

bool CenterLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	result = widget->getPreferedSize();
	return true;
}

void CenterLayout::update(Widget* widget)
{
	Widget* child = widget->getFirstChild();
	if (child)
	{
		Rect rc = widget->getInnerRect();
		Size sz = child->getPreferedSize();

		float ratio = float(sz.cx) / sz.cy;

		if (sz.cx > rc.getWidth())
		{
			sz.cx = rc.getWidth();
			sz.cy = int(sz.cx / ratio);
		}

		if (sz.cy > rc.getHeight())
		{
			sz.cy = rc.getHeight();
			sz.cx = int(sz.cy * ratio);
		}

		int x = (m_centerAxis & CaHorizontal) != 0 ? (rc.getWidth() - sz.cx) / 2 : 0;
		int y = (m_centerAxis & CaVertical) != 0 ? (rc.getHeight() - sz.cy) / 2 : 0;

		child->setRect(Rect(x, y, x + sz.cx, y + sz.cy));
	}
}

		}
	}
}
