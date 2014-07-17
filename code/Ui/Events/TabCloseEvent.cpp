#include "Ui/Events/TabCloseEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabCloseEvent", TabCloseEvent, CloseEvent)

TabCloseEvent::TabCloseEvent(EventSubject* sender, TabPage* tabPage)
:	CloseEvent(sender)
,	m_tabPage(tabPage)
{
}

TabPage* TabCloseEvent::getTabPage() const
{
	return m_tabPage;
}

	}
}
