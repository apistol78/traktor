#ifndef traktor_ui_ListViewItemActivateEvent_H
#define traktor_ui_ListViewItemActivateEvent_H

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

class ListViewItem;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS ListViewItemActivateEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	ListViewItemActivateEvent(EventSubject* sender, ListViewItem* item);

	ListViewItem* getItem() const;

private:
	Ref< ListViewItem > m_item;
};
	
	}
}

#endif	// traktor_ui_ListViewItemActivateEvent_H
