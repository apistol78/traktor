#ifndef traktor_ui_TreeViewEditEvent_H
#define traktor_ui_TreeViewEditEvent_H

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

class TreeViewItem;

/*! \brief TreeView edit event.
 * \ingroup UI
 */
class T_DLLCLASS TreeViewEditEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	TreeViewEditEvent(EventSubject* sender, TreeViewItem* item);

	TreeViewItem* getItem() const;

	void cancel();
	
	bool cancelled() const;	

private:
	Ref< TreeViewItem > m_item;
	bool m_cancelled;
};
	
	}
}

#endif	// traktor_ui_TreeViewEditEvent_H
