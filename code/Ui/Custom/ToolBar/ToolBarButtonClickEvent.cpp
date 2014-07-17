#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarButtonClickEvent", ToolBarButtonClickEvent, ButtonClickEvent)

ToolBarButtonClickEvent::ToolBarButtonClickEvent(EventSubject* sender, ToolBarItem* item, const Command& command)
:	ButtonClickEvent(sender, command)
,	m_item(item)
{
}

ToolBarItem* ToolBarButtonClickEvent::getItem() const
{
	return m_item;
}

		}
	}
}
