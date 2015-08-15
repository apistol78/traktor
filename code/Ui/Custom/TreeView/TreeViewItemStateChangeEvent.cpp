#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Custom/TreeView/TreeViewItemStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewItemStateChangeEvent", TreeViewItemStateChangeEvent, Event)

TreeViewItemStateChangeEvent::TreeViewItemStateChangeEvent(EventSubject* sender, TreeViewItem* item)
:	Event(sender)
,	m_item(item)
{
}

TreeViewItem* TreeViewItemStateChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
