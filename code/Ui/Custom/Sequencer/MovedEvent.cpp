#include "Ui/Custom/Sequencer/MovedEvent.h"
#include "Ui/Custom/Sequencer/SequenceItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.MovedEvent", MovedEvent, Event)

MovedEvent::MovedEvent(EventSubject* sender, SequenceItem* item, int32_t movedTo)
:	Event(sender, item)
,	m_movedTo(movedTo)
{
}

		}
	}
}
