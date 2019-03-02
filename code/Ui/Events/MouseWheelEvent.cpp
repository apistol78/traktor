#include "Ui/Events/MouseWheelEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseWheelEvent", MouseWheelEvent, Event)

MouseWheelEvent::MouseWheelEvent(EventSubject* sender, int32_t rotation, const ui::Point& position)
:	Event(sender)
,	m_rotation(rotation)
,	m_position(position)
{
}

int32_t MouseWheelEvent::getRotation() const
{
	return m_rotation;
}

const ui::Point& MouseWheelEvent::getPosition() const
{
	return m_position;
}

	}
}
