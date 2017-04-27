/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/ShowEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShowEvent", ShowEvent, Event)

ShowEvent::ShowEvent(EventSubject* sender, bool visible)
:	Event(sender)
,	m_visible(visible)
{
}

bool ShowEvent::isVisible() const
{
	return m_visible;
}

	}
}
