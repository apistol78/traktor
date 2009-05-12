#include "Ui/Events/CloseEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CloseEvent", CloseEvent, Event)

CloseEvent::CloseEvent(EventSubject* sender, Object* item)
:	Event(sender, item)
,	m_cancelled(false)
{
}

void CloseEvent::cancel()
{
	m_cancelled = true;
}

bool CloseEvent::cancelled() const
{
	return m_cancelled;
}

	}
}
