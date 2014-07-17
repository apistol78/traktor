#ifndef traktor_ui_custom_SelectEvent_H
#define traktor_ui_custom_SelectEvent_H

#include "Core/RefArray.h"
#include "Ui/Events/SelectionChangeEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class Node;
class Edge;

/*! \brief Node or edge selection event.
 * \ingroup UIC
 */
class T_DLLCLASS SelectEvent : public SelectionChangeEvent
{
	T_RTTI_CLASS;

public:
	SelectEvent(EventSubject* sender, const RefArray< Node >& nodes, const RefArray< Edge >& edges);

	const RefArray< Node >& getNodes() const;

	const RefArray< Edge >& getEdges() const;

private:
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
};

		}
	}
}

#endif	// traktor_ui_custom_SelectEvent_H
