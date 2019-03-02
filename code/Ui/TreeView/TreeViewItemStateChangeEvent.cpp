#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemStateChangeEvent", TreeViewItemStateChangeEvent, Event)

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
