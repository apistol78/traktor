#include <algorithm>
#include "Ui/Custom/GridView/GridRow.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridRow", GridRow, Object)

GridRow::GridRow(uint32_t initialState)
:	m_state(initialState)
{
}

void GridRow::setState(uint32_t state)
{
	m_state = state;
}

void GridRow::setFont(Font* font)
{
	m_font = font;
}

void GridRow::addItem(GridItem* item)
{
	m_items.push_back(item);
}

void GridRow::addChild(GridRow* row)
{
	T_ASSERT (!row->m_parent);
	m_children.push_back(row);
	row->m_parent = this;
}

void GridRow::insertChildBefore(GridRow* insertBefore, GridRow* row)
{
	T_ASSERT (insertBefore->m_parent == this);
	T_ASSERT (!row->m_parent);
	RefArray< GridRow >::iterator i = std::find(m_children.begin(), m_children.end(), insertBefore);
	T_ASSERT (i != m_children.end());
	m_children.insert(i, row);
	row->m_parent = this;
}

void GridRow::insertChildAfter(GridRow* insertAfter, GridRow* row)
{
	T_ASSERT (insertAfter->m_parent == this);
	T_ASSERT (!row->m_parent);
	RefArray< GridRow >::iterator i = std::find(m_children.begin(), m_children.end(), insertAfter);
	T_ASSERT (i != m_children.end());
	m_children.insert(++i, row);
	row->m_parent = this;
}

void GridRow::removeChild(GridRow* row)
{
	T_ASSERT (row->m_parent == this);
	RefArray< GridRow >::iterator i = std::find(m_children.begin(), m_children.end(), row);
	m_children.erase(i);
	row->m_parent = 0;
}

void GridRow::removeAllChildren()
{
	for (RefArray< GridRow >::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->m_parent = 0;
	m_children.clear();
}

		}
	}
}
