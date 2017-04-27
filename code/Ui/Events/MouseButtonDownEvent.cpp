/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/MouseButtonDownEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseButtonDownEvent", MouseButtonDownEvent, Event)

MouseButtonDownEvent::MouseButtonDownEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t MouseButtonDownEvent::getButton() const
{
	return m_button;
}

const ui::Point& MouseButtonDownEvent::getPosition() const
{
	return m_position;
}

	}
}
