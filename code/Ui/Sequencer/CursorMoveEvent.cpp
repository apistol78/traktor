#include "Ui/Sequencer/CursorMoveEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CursorMoveEvent", CursorMoveEvent, Event)

CursorMoveEvent::CursorMoveEvent(EventSubject* sender, int32_t position)
:	Event(sender)
,	m_position(position)
{
}

int32_t CursorMoveEvent::getPosition() const
{
	return m_position;
}

	}
}
