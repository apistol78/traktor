#include "Ui/Events/NcMouseMoveEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NcMouseMoveEvent", NcMouseMoveEvent, Event)

NcMouseMoveEvent::NcMouseMoveEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t NcMouseMoveEvent::getButton() const
{
	return m_button;
}

const ui::Point& NcMouseMoveEvent::getPosition() const
{
	return m_position;
}

	}
}
