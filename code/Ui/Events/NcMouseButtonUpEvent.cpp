/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/NcMouseButtonUpEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NcMouseButtonUpEvent", NcMouseButtonUpEvent, Event)

NcMouseButtonUpEvent::NcMouseButtonUpEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t NcMouseButtonUpEvent::getButton() const
{
	return m_button;
}

const ui::Point& NcMouseButtonUpEvent::getPosition() const
{
	return m_position;
}

	}
}
