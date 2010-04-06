#include <stack>
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/GridView/GridCell.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridHeaderCell.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_headerSize = 24;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridView", GridView, AutoWidget)

GridView::GridView()
:	m_clickColumn(-1)
{
}

bool GridView::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style))
		return false;

	setBackgroundColor(getSystemColor(ScWindowBackground));

	addButtonDownEventHandler(ui::createMethodHandler(this, &GridView::eventButtonDown));
	addButtonUpEventHandler(ui::createMethodHandler(this, &GridView::eventButtonUp));

	m_headerCell = new GridHeaderCell();
	return true;
}

void GridView::addColumn(GridColumn* column)
{
	m_columns.push_back(column);
	requestLayout();
}

int32_t GridView::getColumnIndex(int32_t x) const
{
	int32_t left = 0;
	for (RefArray< GridColumn >::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
	{
		int32_t right = left + (*i)->getWidth();
		if (x >= left && x <= right)
			return int32_t(std::distance(m_columns.begin(), i));
		left = right;
	}
	return -1;
}

void GridView::addRow(GridRow* row)
{
	m_rows.push_back(row);
	requestLayout();
}

void GridView::removeAllRows()
{
	m_rows.resize(0);
	requestLayout();
}

const RefArray< GridRow >& GridView::getRows() const
{
	return m_rows;
}

uint32_t GridView::getRows(RefArray< GridRow >& outRows, uint32_t flags) const
{
	typedef std::pair< RefArray< GridRow >::const_iterator, RefArray< GridRow >::const_iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_rows.begin(), m_rows.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			GridRow* row = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (row->getState() & GridRow::RsSelected)
					outRows.push_back(row);
			}
			else
				outRows.push_back(row);

			if (flags & GfDescendants)
			{
				if ((flags & GfExpandedOnly) != GfExpandedOnly || (row->getState() & GridRow::RsExpanded) == GridRow::RsExpanded)
				{
					const RefArray< GridRow >& children = row->getChildren();
					if (!children.empty())
						stack.push(std::make_pair(children.begin(), children.end()));
				}
			}
		}
		else
			stack.pop();
	}

	return uint32_t(outRows.size());
}

GridRow* GridView::getSelectedRow() const
{
	RefArray< GridRow > selectedRows;
	if (getRows(selectedRows, GfDescendants | GfSelectedOnly) == 1)
		return selectedRows[0];
	else
		return 0;
}

void GridView::addSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

void GridView::addClickEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiClick, eventHandler);
}

void GridView::addExpandEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiExpand, eventHandler);
}

void GridView::layoutCells(const Rect& rc)
{
	Rect rcLayout = rc;

	if (m_headerCell)
	{
		m_headerCell->setColumns(m_columns);

		Rect rcHeader(rcLayout.left, rcLayout.top, rcLayout.right, rcLayout.top + c_headerSize);
		placeCell(m_headerCell, rcHeader);

		rcLayout.top += c_headerSize;
	}

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

	Rect rcRow(rcLayout.left, rcLayout.top, rcLayout.right, rcLayout.top);
	for (RefArray< GridRow >::iterator i = rows.begin(); i != rows.end(); ++i)
	{
		int32_t rowHeight = (*i)->getHeight();

		rcRow.bottom = rcRow.top + rowHeight;
		(*i)->placeCells(this, rcRow, m_columns);

		rcRow.top = rcRow.bottom;
	}
}

void GridView::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	const Point& position = mouseEvent->getPosition();

	m_clickRow = 0;
	m_clickColumn = -1;

	// De-select all rows if no modifier key.
	bool modifier = bool((mouseEvent->getKeyState() & (KsShift | KsControl)) != 0);
	if (!modifier)
	{
		RefArray< GridRow > rows;
		getRows(rows, GfDescendants);
		for (RefArray< GridRow >::iterator i = rows.begin(); i != rows.end(); ++i)
			(*i)->setState((*i)->getState() & ~GridRow::RsSelected);
	}

	// Check for row click; move selection.
	AutoWidgetCell* cell = hitTest(position);
	if (GridRow* row = dynamic_type_cast< GridRow* >(cell))
	{
		// Select picked row.
		row->setState(row->getState() | GridRow::RsSelected);

		// Save column index.
		m_clickRow = row;
		m_clickColumn = getColumnIndex(position.x);
	}

	raiseEvent(EiSelectionChange, 0);
	requestUpdate();
}

void GridView::eventButtonUp(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	const Point& position = mouseEvent->getPosition();

	AutoWidgetCell* cell = hitTest(position);
	if (GridRow* row = dynamic_type_cast< GridRow* >(cell))
	{
		// If still same column index then user clicked on column.
		if (m_clickColumn != -1 && m_clickColumn == getColumnIndex(position.x))
		{
			CommandEvent cmdEvent(this, m_clickRow, Command(m_clickColumn));
			raiseEvent(EiClick, &cmdEvent);

			m_clickRow = 0;
			m_clickColumn = -1;
		}
	}
}

		}
	}
}
