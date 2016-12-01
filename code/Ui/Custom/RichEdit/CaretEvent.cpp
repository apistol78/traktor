#include "Ui/Custom/RichEdit/CaretEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.CaretEvent", CaretEvent, Event)

CaretEvent::CaretEvent(EventSubject* sender)
:	Event(sender)
{
}

		}
	}
}
