#include "Ui/Events/NcMouseButtonDownEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NcMouseButtonDownEvent", NcMouseButtonDownEvent, Event)

NcMouseButtonDownEvent::NcMouseButtonDownEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t NcMouseButtonDownEvent::getButton() const
{
	return m_button;
}

const ui::Point& NcMouseButtonDownEvent::getPosition() const
{
	return m_position;
}

	}
}
