#include "Ui/Events/TreeViewContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewContentChangeEvent", TreeViewContentChangeEvent, ContentChangeEvent)

TreeViewContentChangeEvent::TreeViewContentChangeEvent(EventSubject* sender, TreeViewItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

TreeViewItem* TreeViewContentChangeEvent::getItem() const
{
	return m_item;
}

	}
}
