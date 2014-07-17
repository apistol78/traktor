#include "Ui/Events/TimerEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TimerEvent", TimerEvent, Event)

TimerEvent::TimerEvent(EventSubject* sender, uint32_t id)
:	Event(sender)
,	m_id(id)
{
}

uint32_t TimerEvent::getId() const
{
	return m_id;
}

	}
}
