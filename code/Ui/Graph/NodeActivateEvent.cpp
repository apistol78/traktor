#include "Ui/Graph/NodeActivateEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NodeActivateEvent", NodeActivateEvent, Event)

NodeActivateEvent::NodeActivateEvent(EventSubject* sender, Node* node)
:	Event(sender)
,	m_node(node)
{
}

Node* NodeActivateEvent::getNode() const
{
	return m_node;
}

	}
}
