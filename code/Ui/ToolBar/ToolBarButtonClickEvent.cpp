#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarButtonClickEvent", ToolBarButtonClickEvent, ButtonClickEvent)

ToolBarButtonClickEvent::ToolBarButtonClickEvent(EventSubject* sender, ToolBarItem* item, const Command& command, const MenuItem* menuItem)
:	ButtonClickEvent(sender, command)
,	m_item(item)
,	m_menuItem(menuItem)
{
}

ToolBarItem* ToolBarButtonClickEvent::getItem() const
{
	return m_item;
}

const MenuItem* ToolBarButtonClickEvent::getMenuItem() const
{
	return m_menuItem;
}

	}
}
