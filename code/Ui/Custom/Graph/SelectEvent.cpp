#include "Ui/Custom/Graph/SelectEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SelectEvent", SelectEvent, Event)

SelectEvent::SelectEvent(EventSubject* sender, const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	Event(sender, 0)
,	m_nodes(nodes)
,	m_edges(edges)
{
}

const RefArray< Node >& SelectEvent::getNodes() const
{
	return m_nodes;
}

const RefArray< Edge >& SelectEvent::getEdges() const
{
	return m_edges;
}

		}
	}
}
