#include "Ui/Custom/GridView/GridColumn.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridColumn", GridColumn, Object)

GridColumn::GridColumn(const std::wstring& title, uint32_t width)
:	m_title(title)
,	m_width(width)
{
}

void GridColumn::setTitle(const std::wstring& title)
{
	m_title = title;
}

void GridColumn::setWidth(uint32_t width)
{
	m_width = width;
}

		}
	}
}
