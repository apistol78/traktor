#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CommandEvent", CommandEvent, Event)

CommandEvent::CommandEvent(EventSubject* sender, const Command& command)
:	Event(sender)
,	m_command(command)
{
}

CommandEvent::CommandEvent(EventSubject* sender)
:	Event(sender)
{
}

const Command& CommandEvent::getCommand() const
{
	return m_command;
}

	}
}
