/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/MouseButtonUpEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseButtonUpEvent", MouseButtonUpEvent, Event)

MouseButtonUpEvent::MouseButtonUpEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t MouseButtonUpEvent::getButton() const
{
	return m_button;
}

const ui::Point& MouseButtonUpEvent::getPosition() const
{
	return m_position;
}

	}
}
