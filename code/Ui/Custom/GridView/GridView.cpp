#include <stack>
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/Custom/GridView/GridCell.h"
#include "Ui/Custom/GridView/GridCellContentChangeEvent.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridColumnClickEvent.h"
#include "Ui/Custom/GridView/GridHeaderCell.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridRowDoubleClickEvent.h"
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

struct SortRowPredicateLexical
{
	int32_t columnIndex;
	bool ascending;

	SortRowPredicateLexical(int32_t columnIndex_, bool ascending_)
	:	columnIndex(columnIndex_)
	,	ascending(ascending_)
	{
	}

	bool operator () (const GridRow* row1, const GridRow* row2) const
	{
		const GridCell* cell1 = row1->get().at(columnIndex);
		const GridCell* cell2 = row2->get().at(columnIndex);
		int32_t cmp = cell1->getText().compare(cell2->getText());
		if (cmp < 0)
			return !ascending;
		else
			return ascending;
	}
};

struct SortRowPredicateNumerical
{
	int32_t columnIndex;
	bool ascending;

	SortRowPredicateNumerical(int32_t columnIndex_, bool ascending_)
	:	columnIndex(columnIndex_)
	,	ascending(ascending_)
	{
	}

	bool operator () (const GridRow* row1, const GridRow* row2) const
	{
		const GridCell* cell1 = row1->get().at(columnIndex);
		const GridCell* cell2 = row2->get().at(columnIndex);
		
		float num1 = parseString< float >(cell1->getText());
		float num2 = parseString< float >(cell2->getText());

		if (num1 < num2)
			return !ascending;
		else
			return ascending;
	}
};

int32_t indexOf(const RefArray< GridRow >& rows, const GridRow* row)
{
	RefArray< GridRow >::const_iterator i = std::find(rows.begin(), rows.end(), row);
	if (i != rows.end())
		return int32_t(std::distance(rows.begin(), i));
	else
		return -1;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridView", GridView, AutoWidget)

GridView::GridView()
:	m_clickColumn(-1)
,	m_sortColumnIndex(-1)
,	m_sortAscending(false)
,	m_sortMode(SmLexical)
{
}

bool GridView::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &GridView::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &GridView::eventButtonUp);
	addEventHandler< MouseDoubleClickEvent >(this, &GridView::eventDoubleClick);

	m_itemEditor = new Edit();
	m_itemEditor->create(this, L"", WsBorder);
	m_itemEditor->hide();
	m_itemEditor->addEventHandler< FocusEvent >(this, &GridView::eventEditFocus);

	if ((style & WsColumnHeader) != 0)
		m_headerCell = new GridHeaderCell();

	return true;
}

void GridView::addColumn(GridColumn* column)
{
	m_columns.push_back(column);
	requestUpdate();
}

const RefArray< GridColumn >& GridView::getColumns() const
{
	return m_columns;
}

void GridView::setSortColumn(int32_t columnIndex, bool ascending, SortMode mode)
{
	m_sortColumnIndex = columnIndex;
	m_sortAscending = ascending;
	m_sortMode = mode;
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
	requestUpdate();
}

void GridView::removeRow(GridRow* row)
{
	m_rows.remove(row);
	requestUpdate();
}

void GridView::removeAllRows()
{
	m_rows.resize(0);
	requestUpdate();
}

GridRow* GridView::getRow(int32_t index)
{
	if (index >= 0 && index < int32_t(m_rows.size()))
		return m_rows[index];
	else
		return 0;
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

void GridView::selectAll()
{
	RefArray< GridRow > rows;
	getRows(rows, GfDescendants);
	for (RefArray< GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
		(*i)->setState((*i)->getState() | GridRow::RsSelected);
	requestUpdate();
}

void GridView::deselectAll()
{
	RefArray< GridRow > rows;
	getRows(rows, GfDescendants);
	for (RefArray< GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
		(*i)->setState((*i)->getState() & ~GridRow::RsSelected);
	requestUpdate();
}

void GridView::layoutCells(const Rect& rc)
{
	Rect rcLayout = rc;

	if (m_headerCell)
	{
		m_headerCell->setColumns(m_columns);

		Rect rcHeader(rcLayout.left, rcLayout.top, rcLayout.right, rcLayout.top + scaleBySystemDPI(c_headerSize));
		placeCell(m_headerCell, rcHeader);

		rcLayout.top += scaleBySystemDPI(c_headerSize);
	}

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

#if !defined(__APPLE__) && !defined(__PNACL__)
	if (m_sortColumnIndex >= 0)
	{
		if (m_sortMode == SmLexical)
		{
			SortRowPredicateLexical sortPredicate(m_sortColumnIndex, m_sortAscending);
			rows.sort(sortPredicate);
		}
		else if (m_sortMode == SmNumerical)
		{
			SortRowPredicateNumerical sortPredicate(m_sortColumnIndex, m_sortAscending);
			rows.sort(sortPredicate);
		}
	}
#endif

	Rect rcRow(rcLayout.left, rcLayout.top, rcLayout.right, rcLayout.top);
	for (RefArray< GridRow >::iterator i = rows.begin(); i != rows.end(); ++i)
	{
		int32_t rowHeight = (*i)->getHeight();

		rcRow.bottom = rcRow.top + rowHeight;
		placeCell(*i, rcRow);

		rcRow.top = rcRow.bottom;
	}
}

void GridView::beginEdit(GridCell* item)
{
	releaseCapturedCell();

	m_itemEditor->setRect(getCellClientRect(item));
	m_itemEditor->setText(item->getText());
	m_itemEditor->selectAll();
	m_itemEditor->show();
	m_itemEditor->setFocus();

	m_editItem = item;
}

void GridView::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus() && m_editItem)
	{
		std::wstring originalText = m_editItem->getText();
		std::wstring newText = m_itemEditor->getText();

		m_itemEditor->hide();

		m_editItem->setText(newText);

		GridCellContentChangeEvent changeEvent(this, m_editItem);
		raiseEvent(&changeEvent);

		if (!changeEvent.consumed())
			m_editItem->setText(originalText);
	}
}

void GridView::eventButtonDown(MouseButtonDownEvent* event)
{
	const Point& position = event->getPosition();
	int32_t state = event->getKeyState();

	// Only allow selection with left mouse button.
	if (event->getButton() != MbtLeft)
		return;

	AutoWidgetCell* cell = hitTest(position);

	// Prevent selection change when expanding row.
	if (GridRow* row = dynamic_type_cast< GridRow* >(cell))
	{
		if (!row->getChildren().empty())
		{
			int32_t rx = row->getDepth() * 16 + 16;
			if (position.x <= rx)
				return;
		}
	}

	// De-select all rows if no modifier key.
	bool modifier = bool((state & (KsShift | KsControl)) != 0);
	if (!modifier)
	{
		RefArray< GridRow > rows;
		getRows(rows, GfDescendants);
		for (RefArray< GridRow >::iterator i = rows.begin(); i != rows.end(); ++i)
			(*i)->setState((*i)->getState() & ~GridRow::RsSelected);
	}

	// Check for row click; move selection.
	if (GridRow* row = dynamic_type_cast< GridRow* >(cell))
	{
		RefArray< GridRow > rows;
		getRows(rows, GfDescendants | GfExpandedOnly);

		// Select range.
		if ((state & KsShift) != 0 && m_clickRow)
		{
			int32_t fromRowIndex = indexOf(rows, m_clickRow);
			int32_t toRowIndex = indexOf(rows, row);
			if (fromRowIndex >= 0 && toRowIndex >= 0)
			{
				if (fromRowIndex > toRowIndex)
					std::swap(fromRowIndex, toRowIndex);

				for (int32_t i = fromRowIndex; i <= toRowIndex; ++i)
					rows[i]->setState(rows[i]->getState() | GridRow::RsSelected);
			}
		}
		else
		{
			// Toggle selection on row.
			if ((row->getState() & GridRow::RsSelected) != 0)
				row->setState(row->getState() & ~GridRow::RsSelected);
			else
				row->setState(row->getState() | GridRow::RsSelected);
		}

		// Save column index.
		m_clickRow = row;
		m_clickColumn = getColumnIndex(position.x);
	}
	else
	{
		// Nothing hit.
		m_clickRow = 0;
		m_clickColumn = -1;
	}

	SelectionChangeEvent selectionChange(this);
	raiseEvent(&selectionChange);
	requestUpdate();
}

void GridView::eventButtonUp(MouseButtonUpEvent* event)
{
	const Point& position = event->getPosition();

	// Only allow click with left mouse button.
	if (event->getButton() != MbtLeft)
		return;

	AutoWidgetCell* cell = hitTest(position);
	if (cell != 0 && is_a< GridRow >(cell))
	{
		// If still same column index then user clicked on column.
		if (m_clickColumn != -1 && m_clickColumn == getColumnIndex(position.x))
		{
			GridColumnClickEvent columnClickEvent(this, m_clickRow, m_clickColumn);
			raiseEvent(&columnClickEvent);
		}
	}
}

void GridView::eventDoubleClick(MouseDoubleClickEvent* event)
{
	const Point& position = event->getPosition();

	// Only allow click with left mouse button.
	if (event->getButton() != MbtLeft)
		return;

	AutoWidgetCell* cell = hitTest(position);
	if (cell != 0 && is_a< GridRow >(cell))
	{
		GridRowDoubleClickEvent rowDoubleClick(
			this,
			checked_type_cast< GridRow* >(cell),
			getColumnIndex(position.x)
		);
		raiseEvent(&rowDoubleClick);
	}
}

		}
	}
}
