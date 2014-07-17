#include "Ui/Events/MenuClickEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuClickEvent", MenuClickEvent, Event)

MenuClickEvent::MenuClickEvent(EventSubject* sender, MenuItem* item, const Command& command)
:	Event(sender)
,	m_item(item)
,	m_command(command)
{
}

MenuItem* MenuClickEvent::getItem() const
{
	return m_item;
}

const Command& MenuClickEvent::getCommand() const
{
	return m_command;
}

	}
}
