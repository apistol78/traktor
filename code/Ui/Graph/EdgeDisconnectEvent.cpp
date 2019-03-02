#include "Ui/Graph/EdgeDisconnectEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EdgeDisconnectEvent", EdgeDisconnectEvent, Event)

EdgeDisconnectEvent::EdgeDisconnectEvent(EventSubject* sender, Edge* edge)
:	Event(sender)
,	m_edge(edge)
{
}

Edge* EdgeDisconnectEvent::getEdge() const
{
	return m_edge;
}

	}
}
