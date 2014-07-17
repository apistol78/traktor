#include "Ui/Events/FocusEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FocusEvent", FocusEvent, Event)

FocusEvent::FocusEvent(EventSubject* sender, bool gotFocus)
:	Event(sender)
,	m_gotFocus(gotFocus)
{
}

bool FocusEvent::gotFocus() const
{
	return m_gotFocus;
}

bool FocusEvent::lostFocus() const
{
	return !m_gotFocus;
}

	}
}
