/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/AspectLayout.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AspectLayout", AspectLayout, Layout)

AspectLayout::AspectLayout(float ratio)
:	m_ratio(ratio)
{
}

bool AspectLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	Widget* child = widget->getFirstChild();
	if (!child)
		return false;

	Size sz = child->getPreferedSize();
	float ratio = m_ratio > 0.0f ? m_ratio : float(sz.cx) / sz.cy;

	int width = bounds.cx;
	int height = int(width / ratio);
	if (height > bounds.cy)
	{
		height = bounds.cy;
		width = int(height * ratio);
	}

	result.cx = width;
	result.cy = height;
	return true;
}

void AspectLayout::update(Widget* widget)
{
	Widget* child = widget->getFirstChild();
	if (child != 0)
	{
		Size sz = child->getPreferedSize();
		float ratio = m_ratio > 0.0f ? m_ratio : float(sz.cx) / sz.cy;

		Rect rc = widget->getInnerRect();

		int width = rc.getWidth();
		int height = int(width / ratio);
		if (height > rc.getHeight())
		{
			height = rc.getHeight();
			width = int(height * ratio);
		}

		int x = rc.left + (rc.getWidth() - width) / 2;
		int y = rc.top + (rc.getHeight() - height) / 2;

		child->setRect(Rect(x, y, x + width, y + height));
	}
}

		}
	}
}
