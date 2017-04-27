/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/SelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SelectionChangeEvent", SelectionChangeEvent, Event)

SelectionChangeEvent::SelectionChangeEvent(EventSubject* sender)
:	Event(sender)
{
}

SelectionChangeEvent::SelectionChangeEvent(EventSubject* sender, Object* item)
:	Event(sender)
,	m_item(item)
{
}

Object* SelectionChangeEvent::getItem() const
{
	return m_item;
}

	}
}
