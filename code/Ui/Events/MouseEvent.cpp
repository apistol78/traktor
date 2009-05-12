#include "Ui/Events/MouseEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseEvent", MouseEvent, Event)

MouseEvent::MouseEvent(EventSubject* sender, Object* item, int button, const ui::Point& position, int wheelRotation)
:	Event(sender, item)
,	m_button(button)
,	m_position(position)
,	m_wheelRotation(wheelRotation)
{
}

int MouseEvent::getButton() const
{
	return m_button;
}

const ui::Point& MouseEvent::getPosition() const
{
	return m_position;
}

int MouseEvent::getWheelRotation() const
{
	return m_wheelRotation;
}

	}
}
