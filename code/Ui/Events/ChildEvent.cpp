#include "Ui/Events/ChildEvent.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ChildEvent", ChildEvent, Event)

ChildEvent::ChildEvent(EventSubject* sender, Widget* child, bool link)
:	Event(sender, child)
,	m_link(link)
{
}

bool ChildEvent::link() const
{
	return m_link;
}

	}
}
