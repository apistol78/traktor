#include "Ui/TreeViewItem.h"
#include "Ui/Events/TreeViewEditEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewEditEvent", TreeViewEditEvent, Event)

TreeViewEditEvent::TreeViewEditEvent(EventSubject* sender, TreeViewItem* item)
:	Event(sender, item)
,	m_cancelled(false)
{
}

void TreeViewEditEvent::cancel()
{
	m_cancelled = true;
}

bool TreeViewEditEvent::cancelled() const
{
	return m_cancelled;
}

	}
}
