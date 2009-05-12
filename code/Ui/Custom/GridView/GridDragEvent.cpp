#include "Ui/Custom/GridView/GridDragEvent.h"
#include "Ui/Custom/GridView/GridRow.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridDragEvent", GridDragEvent, Object)

GridDragEvent::GridDragEvent(EventSubject* sender, GridRow* dragItem, int dropRow)
:	Event(sender, dragItem)
,	m_dropRow(dropRow)
,	m_cancelled(false)
{
}

int GridDragEvent::getDropRow() const
{
	return m_dropRow;
}

void GridDragEvent::cancel()
{
	m_cancelled = true;
}

bool GridDragEvent::cancelled() const
{
	return m_cancelled;
}

		}
	}
}
