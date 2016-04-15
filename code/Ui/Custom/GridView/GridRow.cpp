#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Event.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridRowStateChangeEvent.h"
#include "Ui/Custom/GridView/GridView.h"

// Resources
#include "Resources/GridView.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridRow", GridRow, GridCell)

GridRow::GridRow(uint32_t initialState)
:	m_state(initialState)
,	m_background(255, 255, 255, 0)
,	m_minimumHeight(0)
,	m_parent(0)
{
	m_expand = new ui::StyleBitmap(L"UI.GridView", c_ResourceGridView, sizeof(c_ResourceGridView));
}

GridRow::~GridRow()
{
	for (RefArray< GridCell >::iterator i = m_items.begin(); i != m_items.end(); ++i)
		(*i)->m_row = 0;
}

void GridRow::setState(uint32_t state)
{
	m_state = state;
}

void GridRow::setBackground(const Color4ub& background)
{
	m_background = background;
}

void GridRow::setMinimumHeight(int32_t minimumHeight)
{
	m_minimumHeight = minimumHeight;
}

uint32_t GridRow::add(GridCell* item)
{
	T_ASSERT (item->m_row == 0);
	item->m_row = this;
	m_items.push_back(item);
	return uint32_t(m_items.size() - 1);
}

void GridRow::set(uint32_t index, GridCell* item)
{
	if (index < m_items.size())
	{
		if (m_items[index])
		{
			T_ASSERT (m_items[index]->m_row == this);
			m_items[index]->m_row = 0;
		}
		T_ASSERT (item->m_row == 0);
		item->m_row = this;
		m_items[index] = item;
	}
}

Ref< GridCell > GridRow::get(uint32_t index) const
{
	return index < m_items.size() ? m_items[index] : 0;
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

void GridRow::placeCells(AutoWidget* widget, const Rect& rect)
{
	GridView* gridView = checked_type_cast< GridView*, false >(widget);
	const RefArray< GridColumn >& columns = gridView->getColumns();

	// Distribute column cells.
	int32_t depth = getDepth();
	if (!m_children.empty())
		++depth;

	int32_t size = m_expand->getSize().cy;

	Rect rcCell(rect.left, rect.top, rect.left, rect.bottom);
	for (uint32_t i = 0; i < columns.size(); ++i)
	{
		if (i >= m_items.size())
			break;

		int32_t width = columns[i]->getWidth();
		if (columns.size() == 1)
			width = rect.getWidth();

		rcCell.right = rcCell.left + width;

		Rect rcCellLocal = rcCell;
		if (i == 0)
			rcCellLocal.left += depth * size;
		widget->placeCell(m_items[i], rcCellLocal);

		rcCell.left = rcCell.right;
	}

	AutoWidgetCell::placeCells(widget, rect);
}

void GridRow::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	// Handle expand/collapse.
	if (!m_children.empty())
	{
		int32_t depth = getDepth();
		int32_t size = m_expand->getSize().cy;
		int32_t rx = depth * size + size;
		if (position.x <= rx)
		{
			if (m_state & RsExpanded)
				m_state &= ~RsExpanded;
			else
				m_state |= RsExpanded;

			GridRowStateChangeEvent expandEvent(getWidget(), this);
			getWidget()->raiseEvent(&expandEvent);
			getWidget()->requestUpdate();
		}
	}
}

void GridRow::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	GridView* gridView = checked_type_cast< GridView*, false >(getWidget());

	const RefArray< GridColumn >& columns = gridView->getColumns();
	Rect rowRect(0, rect.top, rect.getWidth(), rect.bottom);

	// Paint custom background.
	if (m_background.a > 0)
	{
		canvas.setBackground(m_background);
		canvas.fillRect(rowRect);
	}

	// Paint selection background.
	if (m_state & GridRow::RsSelected)
	{
		canvas.setBackground(ss->getColor(gridView, L"item-background-color-selected"));
		canvas.fillRect(rowRect);
	}

	if (!m_children.empty())
	{
		int32_t depth = getDepth();
		int32_t size = m_expand->getSize().cy;
		canvas.drawBitmap(
			Point(rect.left + 2 + depth * size, rect.top + (rect.getHeight() - size) / 2),
			Point((m_state & GridRow::RsExpanded) ? size : 0, 0),
			Size(size, size),
			m_expand,
			BmAlpha
		);
	}

	canvas.setForeground(ss->getColor(gridView, L"line-color"));

	if (columns.size() >= 2)
	{
		int32_t left = rect.left;
		for (RefArray< GridColumn >::const_iterator i = columns.begin(); i != columns.end() - 1; ++i)
		{
			left += (*i)->getWidth();
			canvas.drawLine(left, rect.top, left, rect.bottom - 1);
		}
	}

	canvas.drawLine(0, rect.bottom - 1, rect.getWidth(), rect.bottom - 1);
}

int32_t GridRow::getHeight() const
{
	int32_t rowHeight = m_minimumHeight;
	for (RefArray< GridCell >::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
		rowHeight = std::max(rowHeight, (*i)->getHeight());
	return rowHeight;
}

void GridRow::setText(const std::wstring& text)
{
}

std::wstring GridRow::getText() const
{
	return L"";
}

bool GridRow::edit()
{
	return false;
}

int32_t GridRow::getDepth() const
{
	int32_t depth = 0;
	for (GridRow* row = m_parent; row; row = row->m_parent)
		++depth;
	return depth;
}

		}
	}
}
