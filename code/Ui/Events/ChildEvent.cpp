#include "Ui/Widget.h"
#include "Ui/Events/ChildEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ChildEvent", ChildEvent, Event)

ChildEvent::ChildEvent(EventSubject* sender, Widget* parent, Widget* child, bool link)
:	Event(sender)
,	m_parent(parent)
,	m_child(child)
,	m_link(link)
{
}

Widget* ChildEvent::getParent() const
{
	return m_parent;
}

Widget* ChildEvent::getChild() const
{
	return m_child;
}

bool ChildEvent::link() const
{
	return m_link;
}

	}
}
