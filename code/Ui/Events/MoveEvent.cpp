#include "Ui/Events/MoveEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MoveEvent", MoveEvent, Event)

MoveEvent::MoveEvent(EventSubject* sender, Object* item, const Point& position)
:	Event(sender, item)
,	m_position(position)
{
}

const Point& MoveEvent::getPosition() const
{
	return m_position;
}

	}
}
