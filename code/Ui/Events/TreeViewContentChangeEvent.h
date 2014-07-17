#ifndef traktor_ui_TreeViewContentChangeEvent_H
#define traktor_ui_TreeViewContentChangeEvent_H

#include "Ui/Events/ContentChangeEvent.h"

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
	
/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS TreeViewContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	TreeViewContentChangeEvent(EventSubject* sender, TreeViewItem* item);

	TreeViewItem* getItem() const;

private:
	Ref< TreeViewItem > m_item;
};
	
	}
}

#endif	// traktor_ui_TreeViewContentChangeEvent_H
