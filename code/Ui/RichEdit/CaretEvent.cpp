#include "Ui/RichEdit/CaretEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CaretEvent", CaretEvent, Event)

CaretEvent::CaretEvent(EventSubject* sender)
:	Event(sender)
{
}

	}
}
