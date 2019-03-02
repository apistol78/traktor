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

class Node;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS NodeActivateEvent : public Event
{
	T_RTTI_CLASS;

public:
	NodeActivateEvent(EventSubject* sender, Node* node);

	Node* getNode() const;

private:
	Ref< Node > m_node;
};

	}
}

