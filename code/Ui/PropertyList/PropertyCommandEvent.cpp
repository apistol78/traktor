#include "Ui/PropertyList/PropertyCommandEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertyCommandEvent", PropertyCommandEvent, CommandEvent)

PropertyCommandEvent::PropertyCommandEvent(EventSubject* sender, PropertyItem* item, const Command& command)
:	CommandEvent(sender, command)
,	m_item(item)
{
}

PropertyItem* PropertyCommandEvent::getItem() const
{
	return m_item;
}

	}
}
