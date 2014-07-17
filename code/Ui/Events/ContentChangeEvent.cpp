#include "Ui/Events/ContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ContentChangeEvent", ContentChangeEvent, Event)

ContentChangeEvent::ContentChangeEvent(EventSubject* sender)
:	Event(sender)
{
}

	}
}
