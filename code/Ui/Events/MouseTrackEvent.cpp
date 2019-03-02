#include "Ui/Events/MouseTrackEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseTrackEvent", MouseTrackEvent, Event)

MouseTrackEvent::MouseTrackEvent(EventSubject* sender, bool entered)
:	Event(sender)
,	m_entered(entered)
{
}

bool MouseTrackEvent::entered() const
{
	return m_entered;
}

	}
}
