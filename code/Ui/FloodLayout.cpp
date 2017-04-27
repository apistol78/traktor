/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/FloodLayout.h"
#include "Ui/Widget.h"
#include "Ui/Rect.h"

namespace traktor
{
	namespace ui
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

	result = child->getPreferedSize() + m_margin + m_margin;
	return true;
}

void FloodLayout::update(Widget* widget)
{
	Ref< Widget > child = getFirstVisibleChild(widget);
	if (!child)
		return;

	Rect rc = widget->getInnerRect().inflate(-m_margin.cx, -m_margin.cy);
	child->setRect(rc);
}

Ref< Widget > FloodLayout::getFirstVisibleChild(Widget* widget)
{
	for (Ref< Widget > child = widget->getFirstChild(); child; child = child->getNextSibling())
	{
		if (child->acceptLayout() && child->isVisible(false))
			return child;
	}
	return 0;
}

	}
}
