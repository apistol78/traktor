#include "Ui/Custom/GridView/GridColumnClickEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridColumnClickEvent", GridColumnClickEvent, Event)

GridColumnClickEvent::GridColumnClickEvent(EventSubject* sender, GridRow* row, int32_t column)
:	Event(sender)
,	m_row(row)
,	m_column(column)
{
}

GridRow* GridColumnClickEvent::getRow() const
{
	return m_row;
}

int32_t GridColumnClickEvent::getColumn() const
{
	return m_column;
}

		}
	}
}
