#include "Ui/Events/MoveEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MoveEvent", MoveEvent, Event)

MoveEvent::MoveEvent(EventSubject* sender, const Point& position)
:	Event(sender)
,	m_position(position)
{
}

const Point& MoveEvent::getPosition() const
{
	return m_position;
}

	}
}
