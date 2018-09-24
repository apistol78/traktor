/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NodeMovedEvent_H
#define traktor_ui_NodeMovedEvent_H

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

/*! \brief Node or edge selection event.
 * \ingroup UI
 */
class T_DLLCLASS NodeMovedEvent : public Event
{
	T_RTTI_CLASS;

public:
	NodeMovedEvent(EventSubject* sender, Node* node);

	Node* getNode() const;

private:
	Ref< Node > m_node;
};

	}
}

#endif	// traktor_ui_NodeMovedEvent_H
