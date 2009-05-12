#include "Ui/Events/IdleEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.IdleEvent", IdleEvent, Event)

IdleEvent::IdleEvent(EventSubject* sender)
:	Event(sender, 0)
,	m_requestMore(false)
{
}

void IdleEvent::requestMore()
{
	m_requestMore = true;
}

bool IdleEvent::requestedMore() const
{
	return m_requestMore;
}

	}
}
