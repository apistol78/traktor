/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/GridView/GridRowStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridRowStateChangeEvent", GridRowStateChangeEvent, Event)

GridRowStateChangeEvent::GridRowStateChangeEvent(EventSubject* sender, GridRow* row)
:	Event(sender)
,	m_row(row)
{
}

GridRow* GridRowStateChangeEvent::getRow() const
{
	return m_row;
}

	}
}
