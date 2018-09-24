/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/GridView/GridColumnClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridColumnClickEvent", GridColumnClickEvent, Event)

GridColumnClickEvent::GridColumnClickEvent(EventSubject* sender, GridRow* row, int32_t column)
:	Event(sender)
,	m_row(row)
,	m_column(column)
{
}

GridRow* GridColumnClickEvent::getRow() const
{
	return m_row;
}

int32_t GridColumnClickEvent::getColumn() const
{
	return m_column;
}

	}
}
