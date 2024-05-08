/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stack>
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/StyleBitmap.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridColumnClickEvent.h"
#include "Ui/GridView/GridHeader.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/HierarchicalState.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_headerMargin = 2_ut;

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
		const GridItem* item1 = row1->get(columnIndex);
		const GridItem* item2 = row2->get(columnIndex);
		int32_t cmp = compareIgnoreCase(item1->getText(), item2->getText());
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
		const GridItem* item1 = row1->get(columnIndex);
		const GridItem* item2 = row2->get(columnIndex);

		float num1 = parseString< float >(item1->getText());
		float num2 = parseString< float >(item2->getText());

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

std::wstring getRowPath(GridRow* row)
{
	if (row->get().empty())
		return L"";
	if (row->getParent() != nullptr)
		return getRowPath(row->getParent()) + L"/" + row->get(0)->getText();
	else
		return row->get(0)->getText();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridView", GridView, AutoWidget)

GridView::GridView()
:	m_clickColumn(-1)
,	m_sortColumnIndex(-1)
,	m_sortAscending(false)
,	m_sortMode(SmLexical)
,	m_autoEdit(false)
,	m_multiSelect(false)
{
}

bool GridView::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style))
		return false;

	m_autoEdit = bool((style & WsAutoEdit) == WsAutoEdit);
	m_multiSelect = bool((style & WsMultiSelect) == WsMultiSelect);

	addEventHandler< MouseButtonDownEvent >(this, &GridView::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &GridView::eventButtonUp);
	addEventHandler< MouseDoubleClickEvent >(this, &GridView::eventDoubleClick);

	m_itemEditor = new Edit();
	m_itemEditor->create(this, L"", WsBorder | WsWantAllInput);
	m_itemEditor->hide();
	m_itemEditor->addEventHandler< FocusEvent >(this, &GridView::eventEditFocus);
	m_itemEditor->addEventHandler< ui::KeyDownEvent >(this, &GridView::eventEditKey);

	if ((style & WsColumnHeader) != 0)
		m_header = new GridHeader();

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

GridColumn* GridView::getColumn(uint32_t index) const
{
	return m_columns[index];
}

void GridView::setSortColumn(int32_t columnIndex, bool ascending, SortMode mode)
{
	m_sortColumnIndex = columnIndex;
	m_sortAscending = ascending;
	m_sortMode = mode;
}

void GridView::setSort(const sort_fn_t& sortFn)
{
	m_sortFn = sortFn;
}

int32_t GridView::getColumnIndex(int32_t x) const
{
	int32_t left = 0;
	for (auto i = m_columns.begin(); i != m_columns.end(); ++i)
	{
		int32_t right = left + pixel((*i)->getWidth());
		if (x >= left && x <= right)
			return int32_t(std::distance(m_columns.begin(), i));
		left = right;
	}
	return -1;
}

void GridView::addRow(GridRow* row)
{
	row->setOwner(this);
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
		return nullptr;
}

const RefArray< GridRow >& GridView::getRows() const
{
	return m_rows;
}

RefArray< GridRow > GridView::getRows(uint32_t flags) const
{
	typedef std::pair< RefArray< GridRow >::const_iterator, RefArray< GridRow >::const_iterator > range_t;

	RefArray< GridRow > rows;

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
				if (row->getState() & GridRow::Selected)
					rows.push_back(row);
			}
			else
				rows.push_back(row);

			if (flags & GfDescendants)
			{
				if ((flags & GfExpandedOnly) != GfExpandedOnly || (row->getState() & GridRow::Expanded) == GridRow::Expanded)
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

	return rows;
}

GridRow* GridView::getSelectedRow() const
{
	RefArray< GridRow > selectedRows = getRows(GfDescendants | GfSelectedOnly);
	if (selectedRows.size() == 1)
		return selectedRows[0];
	else
		return nullptr;
}

void GridView::selectAll()
{
	for (auto row : getRows(GfDescendants))
		row->setState(row->getState() | GridRow::Selected);
	requestUpdate();
}

void GridView::deselectAll()
{
	for (auto row : getRows(GfDescendants))
		row->setState(row->getState() & ~GridRow::Selected);
	requestUpdate();
}

void GridView::setMultiSelect(bool multiSelect)
{
	m_multiSelect = multiSelect;
}

void GridView::fitColumn(int32_t columnIndex)
{
	if (columnIndex < 0 || columnIndex >= (int32_t)m_columns.size())
		return;

	const auto fm = getFontMetric();

	int maxWidth = pixel(16_ut);
	for (auto row : getRows(GfDescendants))
	{
		const GridItem* item = row->get(columnIndex);
		if (item)
		{
			const int32_t width = fm.getExtent(item->getText()).cx;
			maxWidth = std::max(maxWidth, width);
		}
	}

	m_columns[columnIndex]->setWidth(unit(maxWidth) + 4_ut);
	requestUpdate();
}

Ref< HierarchicalState > GridView::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();

	for (uint32_t i = 0; i < (uint32_t)m_columns.size(); ++i)
		state->setValue(i, m_columns[i]->getWidth().get());

	RefArray< GridRow > rows = getRows(GfDescendants);
	for (auto row : rows)
	{
		state->addState(
			getRowPath(row),
			(row->getState() & GridRow::Expanded) != 0,
			(row->getState() & GridRow::Selected) != 0
		);
	}

	return state;
}

void GridView::applyState(const HierarchicalState* state)
{
	for (uint32_t i = 0; i < (uint32_t)m_columns.size(); ++i)
	{
		const int32_t width = state->getValue(i, m_columns[i]->getWidth().get());
		m_columns[i]->setWidth(Unit(width));
	}

	RefArray< GridRow > rows = getRows(GfDescendants);
	for (auto row : rows)
	{
		const std::wstring path = getRowPath(row);
		row->setState(
			(state->getExpanded(path) ? GridRow::Expanded : 0) |
			(state->getSelected(path) ? GridRow::Selected : 0)
		);
	}
}

void GridView::layoutCells(const Rect& rc)
{
	int32_t fontHeight = getFontMetric().getHeight();
	Rect rcLayout = rc;

	if (m_header)
	{
		const int32_t headerHeight = fontHeight + pixel(c_headerMargin) * 2;
		m_header->setColumns(m_columns);
		placeHeaderCell(m_header, headerHeight);
		rcLayout.top += headerHeight;
	}

	RefArray< GridRow > rows = getRows(GfDescendants | GfExpandedOnly);

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

	if (m_sortFn)
		rows.sort(m_sortFn);

	Rect rcRow(rcLayout.left, rcLayout.top, rcLayout.right, rcLayout.top);
	for (auto row : rows)
	{
		const int32_t rowHeight = row->getHeight();
		rcRow.bottom = rcRow.top + rowHeight;
		placeCell(row, rcRow);
		rcRow.top = rcRow.bottom;
	}
}

IBitmap* GridView::getBitmap(const wchar_t* const name)
{
	auto it = m_bitmaps.find(name);
	if (it == m_bitmaps.end())
		m_bitmaps[name] = new ui::StyleBitmap(name);
	return m_bitmaps[name];
}

void GridView::beginEdit(GridItem* item)
{
	releaseCapturedCell();

	m_itemEditor->setRect(item->getClientRect());
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

		if (originalText != newText)
		{
			m_editItem->setText(newText);

			GridItemContentChangeEvent changeEvent(this, m_editItem, originalText);
			raiseEvent(&changeEvent);

			if (!changeEvent.consumed())
				m_editItem->setText(originalText);

			event->consume();
		}
	}
}

void GridView::eventEditKey(KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn)
	{
		m_itemEditor->hide();
	}
	else if (event->getVirtualKey() == ui::VkEscape)
	{
		std::wstring originalText = m_editItem->getText();
		m_itemEditor->setText(originalText);
		m_itemEditor->hide();
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

	// De-select all rows if no modifier key or only single select.
	const bool modifier = bool((state & (KsShift | KsControl)) != 0);
	if (!modifier || !m_multiSelect)
	{
		for (auto row : getRows(GfDescendants))
			row->setState(row->getState() & ~GridRow::Selected);
	}

	// Check for row click; move selection.
	if (GridRow* row = dynamic_type_cast< GridRow* >(cell))
	{
		RefArray< GridRow > rows = getRows(GfDescendants | GfExpandedOnly);

		// Select range.
		if (m_multiSelect && (state & KsShift) != 0 && m_clickRow)
		{
			int32_t fromRowIndex = indexOf(rows, m_clickRow);
			int32_t toRowIndex = indexOf(rows, row);
			if (fromRowIndex >= 0 && toRowIndex >= 0)
			{
				if (fromRowIndex > toRowIndex)
					std::swap(fromRowIndex, toRowIndex);

				for (int32_t i = fromRowIndex; i <= toRowIndex; ++i)
					rows[i]->setState(rows[i]->getState() | GridRow::Selected);
			}
		}
		else
		{
			// Toggle selection on row.
			if ((row->getState() & GridRow::Selected) != 0)
				row->setState(row->getState() & ~GridRow::Selected);
			else
				row->setState(row->getState() | GridRow::Selected);
		}

		// Save column index.
		m_clickRow = row;
		m_clickColumn = getColumnIndex(position.x);
	}
	else
	{
		// Nothing hit.
		m_clickRow = nullptr;
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
	if (cell != nullptr && is_a< GridRow >(cell))
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
	if (cell != nullptr && is_a< GridRow >(cell))
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
