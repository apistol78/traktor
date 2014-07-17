#include "Ui/TreeViewItem.h"
#include "Ui/Events/ListViewItemActivateEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListViewItemActivateEvent", ListViewItemActivateEvent, Event)

ListViewItemActivateEvent::ListViewItemActivateEvent(EventSubject* sender, ListViewItem* item)
:	Event(sender)
,	m_item(item)
{
}

ListViewItem* ListViewItemActivateEvent::getItem() const
{
	return m_item;
}

	}
}
