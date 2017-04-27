/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Custom/Auto/ChildWidgetCell.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ChildWidgetCell", ChildWidgetCell, AutoWidgetCell)

ChildWidgetCell::ChildWidgetCell(Widget* child)
:	m_child(child)
{
}

ChildWidgetCell::~ChildWidgetCell()
{
	safeDestroy(m_child);
}

void ChildWidgetCell::placeCells(AutoWidget* widget, const Rect& rect)
{
	AutoWidgetCell::placeCells(widget, rect);
	m_child->setRect(widget->getCellClientRect(this));
}

Widget* ChildWidgetCell::getChild() const
{
	return m_child;
}

		}
	}
}
