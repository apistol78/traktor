#include "Ui/GridView/GridItemContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridItemContentChangeEvent", GridItemContentChangeEvent, ContentChangeEvent)

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
