#include "Ui/Events/ScrollEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ScrollEvent", ScrollEvent, Event)

ScrollEvent::ScrollEvent(EventSubject* sender, Object* item, int position)
:	Event(sender, item)
,	m_position(position)
{
}

int ScrollEvent::getPosition() const
{
	return m_position;
}

	}
}
