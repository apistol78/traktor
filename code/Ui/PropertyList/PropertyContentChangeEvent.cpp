#include "Ui/PropertyList/PropertyContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertyContentChangeEvent", PropertyContentChangeEvent, ContentChangeEvent)

PropertyContentChangeEvent::PropertyContentChangeEvent(EventSubject* sender, PropertyItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

PropertyItem* PropertyContentChangeEvent::getItem() const
{
	return m_item;
}

	}
}
