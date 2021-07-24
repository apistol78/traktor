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

class Widget;

/*! ChildEvent
 * \ingroup UI
 */
class T_DLLCLASS ChildEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit ChildEvent(EventSubject* sender, Widget* parent, Widget* child, bool link);

	Widget* getParent() const;

	Widget* getChild() const;

	bool link() const;

private:
	Widget* m_parent;
	Widget* m_child;
	bool m_link;
};

	}
}

