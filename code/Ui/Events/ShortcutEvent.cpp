#include "Ui/Events/ShortcutEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShortcutEvent", ShortcutEvent, Event)

ShortcutEvent::ShortcutEvent(EventSubject* sender, const Command& command)
:	Event(sender)
,	m_command(command)
{
}

const Command& ShortcutEvent::getCommand() const
{
	return m_command;
}

	}
}
