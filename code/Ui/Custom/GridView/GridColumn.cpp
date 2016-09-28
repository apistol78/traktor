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
,	m_editable(false)
{
}

GridColumn::GridColumn(const std::wstring& title, uint32_t width, bool editable)
:	m_title(title)
,	m_width(width)
,	m_editable(editable)
{
}

void GridColumn::setTitle(const std::wstring& title)
{
	m_title = title;
}

const std::wstring& GridColumn::getTitle() const
{
	return m_title;
}

void GridColumn::setWidth(uint32_t width)
{
	m_width = width;
}

uint32_t GridColumn::getWidth() const
{
	return m_width;
}

void GridColumn::setEditable(bool editable)
{
	m_editable = editable;
}

bool GridColumn::isEditable() const
{
	return m_editable;
}

		}
	}
}
