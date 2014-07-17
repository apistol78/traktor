#include "Ui/Custom/Sequencer/SequenceItem.h"
#include "Ui/Custom/Sequencer/SequenceMovedEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SequenceMovedEvent", SequenceMovedEvent, Event)

SequenceMovedEvent::SequenceMovedEvent(EventSubject* sender, SequenceItem* item, int32_t movedTo)
:	Event(sender)
,	m_item(item)
,	m_movedTo(movedTo)
{
}

SequenceItem* SequenceMovedEvent::getItem() const
{
	return m_item;
}

int32_t SequenceMovedEvent::getMovedTo() const
{
	return m_movedTo;
}

		}
	}
}
