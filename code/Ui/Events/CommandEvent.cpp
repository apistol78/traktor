#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CommandEvent", CommandEvent, Event)

CommandEvent::CommandEvent(EventSubject* sender, Object* item, const Command& command)
:	Event(sender, item)
,	m_command(command)
{
}

CommandEvent::CommandEvent(EventSubject* sender, Object* item)
:	Event(sender, item)
{
}

const Command& CommandEvent::getCommand() const
{
	return m_command;
}

	}
}
