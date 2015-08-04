#include "Ui/Custom/GridView/GridCellContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridCellContentChangeEvent", GridCellContentChangeEvent, ContentChangeEvent)

GridCellContentChangeEvent::GridCellContentChangeEvent(EventSubject* sender, GridCell* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

GridCell* GridCellContentChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
