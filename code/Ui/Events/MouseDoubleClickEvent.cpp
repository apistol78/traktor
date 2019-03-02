#include "Ui/Events/MouseDoubleClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseDoubleClickEvent", MouseDoubleClickEvent, Event)

MouseDoubleClickEvent::MouseDoubleClickEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t MouseDoubleClickEvent::getButton() const
{
	return m_button;
}

const ui::Point& MouseDoubleClickEvent::getPosition() const
{
	return m_position;
}

	}
}
