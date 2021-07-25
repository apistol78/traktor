#include "Ui/Events/TimerEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TimerEvent", TimerEvent, Event)

TimerEvent::TimerEvent(EventSubject* sender)
:	Event(sender)
{
}

	}
}
