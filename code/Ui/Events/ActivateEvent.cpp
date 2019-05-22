#include "Ui/Events/ActivateEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ActivateEvent", ActivateEvent, Event)

ActivateEvent::ActivateEvent(EventSubject* sender, bool activate)
:	Event(sender)
,   m_activate(activate)
{
}

bool ActivateEvent::activate() const
{
	return m_activate;
}

	}
}
