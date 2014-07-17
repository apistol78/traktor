#ifndef traktor_ui_custom_EdgeConnectEvent_H
#define traktor_ui_custom_EdgeConnectEvent_H

#include "Ui/Event.h"

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

class Edge;

/*! \brief Node or edge selection event.
 * \ingroup UIC
 */
class T_DLLCLASS EdgeConnectEvent : public Event
{
	T_RTTI_CLASS;

public:
	EdgeConnectEvent(EventSubject* sender, Edge* edge);

	Edge* getEdge() const;

private:
	Ref< Edge > m_edge;
};

		}
	}
}

#endif	// traktor_ui_custom_EdgeConnectEvent_H
