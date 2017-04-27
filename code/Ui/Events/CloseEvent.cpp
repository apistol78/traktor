/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/CloseEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CloseEvent", CloseEvent, Event)

CloseEvent::CloseEvent(EventSubject* sender)
:	Event(sender)
,	m_cancelled(false)
{
}

void CloseEvent::cancel()
{
	m_cancelled = true;
}

bool CloseEvent::cancelled() const
{
	return m_cancelled;
}

	}
}
