#include "Ui/Events/ContentChangingEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ContentChangingEvent", ContentChangingEvent, Event)

ContentChangingEvent::ContentChangingEvent(EventSubject* sender)
:	Event(sender)
{
}

	}
}
