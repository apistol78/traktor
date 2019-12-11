#pragma once

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edge;

/*! Node or edge selection event.
 * \ingroup UI
 */
class T_DLLCLASS EdgeDisconnectEvent : public Event
{
	T_RTTI_CLASS;

public:
	EdgeDisconnectEvent(EventSubject* sender, Edge* edge);

	Edge* getEdge() const;

private:
	Ref< Edge > m_edge;
};

	}
}

