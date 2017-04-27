/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/IdleEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.IdleEvent", IdleEvent, Event)

IdleEvent::IdleEvent(EventSubject* sender)
:	Event(sender)
,	m_requestMore(false)
{
}

void IdleEvent::requestMore()
{
	m_requestMore = true;
}

bool IdleEvent::requestedMore() const
{
	return m_requestMore;
}

	}
}
