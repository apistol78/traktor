#include "Ui/Custom/PropertyList/PropertySelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PropertySelectionChangeEvent", PropertySelectionChangeEvent, SelectionChangeEvent)

PropertySelectionChangeEvent::PropertySelectionChangeEvent(EventSubject* sender, PropertyItem* item, int32_t index)
:	SelectionChangeEvent(sender)
,	m_item(item)
,	m_index(index)
{
}

PropertyItem* PropertySelectionChangeEvent::getItem() const
{
	return m_item;
}

int32_t PropertySelectionChangeEvent::getIndex() const
{
	return m_index;
}

		}
	}
}
