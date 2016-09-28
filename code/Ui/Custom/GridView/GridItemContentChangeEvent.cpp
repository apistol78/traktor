#include "Ui/Custom/GridView/GridItemContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItemContentChangeEvent", GridItemContentChangeEvent, ContentChangeEvent)

GridItemContentChangeEvent::GridItemContentChangeEvent(EventSubject* sender, GridItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

GridItem* GridItemContentChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
