#include "Ui/Events/MouseMoveEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseMoveEvent", MouseMoveEvent, Event)

MouseMoveEvent::MouseMoveEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t MouseMoveEvent::getButton() const
{
	return m_button;
}

const ui::Point& MouseMoveEvent::getPosition() const
{
	return m_position;
}

	}
}
