#include "Ui/Events/ButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ButtonClickEvent", ButtonClickEvent, Event)

ButtonClickEvent::ButtonClickEvent(EventSubject* sender, const Command& command)
:	Event(sender)
,	m_command(command)
{
}

ButtonClickEvent::ButtonClickEvent(EventSubject* sender)
:	Event(sender)
{
}

const Command& ButtonClickEvent::getCommand() const
{
	return m_command;
}

	}
}
