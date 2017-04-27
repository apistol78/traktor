/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/TimerEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TimerEvent", TimerEvent, Event)

TimerEvent::TimerEvent(EventSubject* sender, uint32_t id)
:	Event(sender)
,	m_id(id)
{
}

uint32_t TimerEvent::getId() const
{
	return m_id;
}

	}
}
