#include "Ui/Custom/GridView/GridRowStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridRowStateChangeEvent", GridRowStateChangeEvent, Event)

GridRowStateChangeEvent::GridRowStateChangeEvent(EventSubject* sender, GridRow* row)
:	Event(sender)
,	m_row(row)
{
}

GridRow* GridRowStateChangeEvent::getRow() const
{
	return m_row;
}

		}
	}
}
