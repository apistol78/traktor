/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstdlib>
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Event.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowStateChangeEvent.h"
#include "Ui/GridView/GridView.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridRow", GridRow, AutoWidgetCell)

GridRow::GridRow(uint32_t initialState)
:	m_editable(true)
,	m_state(initialState)
,	m_minimumHeight(0)
,	m_parent(0)
,	m_editMode(0)
{
}

GridRow::~GridRow()
{
	for (auto item : m_items)
	{
		if (item)
			item->m_row = nullptr;
	}
}

void GridRow::setEditable(bool editable)
{
	m_editable = editable;
}

void GridRow::setState(uint32_t state)
{
	m_state = state;
}

void GridRow::setBackground(const ColorReference& background)
{
	m_background = background;
}

void GridRow::setMinimumHeight(int32_t minimumHeight)
{
	m_minimumHeight = minimumHeight;
}

int32_t GridRow::getHeight() const
{
	int32_t rowHeight = m_minimumHeight;
	for (auto item : m_items)
	{
		if (item)
			rowHeight = std::max(rowHeight, item->getHeight());
	}
	return rowHeight;
}

uint32_t GridRow::add(GridItem* item)
{
	T_ASSERT(item->m_row == nullptr);
	item->m_row = this;
	item->setOwner(getWidget());
	m_items.push_back(item);
	return uint32_t(m_items.size() - 1);
}

uint32_t GridRow::add(const std::wstring& text)
{
	return add(new GridItem(text));
}

uint32_t GridRow::add(const std::wstring& text, Font* font)
{
	return add(new GridItem(text, font));
}
	
uint32_t GridRow::add(const std::wstring& text, IBitmap* image)
{
	return add(new GridItem(text, image));
}

uint32_t GridRow::add(IBitmap* image)
{
	return add(new GridItem(image));
}

void GridRow::set(uint32_t index, GridItem* item)
{
	m_items.resize(std::max< uint32_t >((uint32_t)m_items.size(), index + 1));
	if (m_items[index])
	{
		T_ASSERT(m_items[index]->m_row == this);
		m_items[index]->m_row = nullptr;
	}
	T_ASSERT(item->m_row == nullptr);
	item->m_row = this;
	item->setOwner(getWidget());
	m_items[index] = item;
}

Ref< GridItem > GridRow::get(uint32_t index) const
{
	return index < m_items.size() ? m_items[index] : nullptr;
}

uint32_t GridRow::getIndex(const GridItem* item) const
{
	RefArray< GridItem >::const_iterator i = std::find(m_items.begin(), m_items.end(), item);
	return (i != m_items.end()) ? std::distance(m_items.begin(), i) : 0;
}

void GridRow::addChild(GridRow* row)
{
	T_ASSERT(!row->m_parent);
	m_children.push_back(row);
	row->setOwner(getWidget());
	row->m_parent = this;
}

void GridRow::insertChildBefore(GridRow* insertBefore, GridRow* row)
{
	T_ASSERT(insertBefore->m_parent == this);
	T_ASSERT(!row->m_parent);
	RefArray< GridRow >::iterator i = std::find(m_children.begin(), m_children.end(), insertBefore);
	T_ASSERT(i != m_children.end());
	m_children.insert(i, row);
	row->setOwner(getWidget());
	row->m_parent = this;
}

void GridRow::insertChildAfter(GridRow* insertAfter, GridRow* row)
{
	T_ASSERT(insertAfter->m_parent == this);
	T_ASSERT(!row->m_parent);
	auto it = std::find(m_children.begin(), m_children.end(), insertAfter);
	T_ASSERT(it != m_children.end());
	m_children.insert(++it, row);
	row->setOwner(getWidget());
	row->m_parent = this;
}

void GridRow::removeChild(GridRow* row)
{
	T_ASSERT(row->m_parent == this);
	auto it = std::find(m_children.begin(), m_children.end(), row);
	m_children.erase(it);
	row->m_parent = nullptr;
}

void GridRow::removeAllChildren()
{
	for (auto child : m_children)
		child->m_parent = nullptr;
	m_children.clear();
}

void GridRow::setOwner(AutoWidget* owner)
{
	setWidget(owner);

	for (auto item : m_items)
		item->setOwner(owner);

	for (auto child : m_children)
		child->setOwner(owner);
}

int32_t GridRow::getDepth() const
{
	int32_t depth = 0;
	for (GridRow* row = m_parent; row; row = row->m_parent)
		++depth;
	return depth;
}

void GridRow::placeCells(AutoWidget* widget, const Rect& rect)
{
	GridView* gridView = checked_type_cast< GridView*, false >(widget);
	const RefArray< GridColumn >& columns = gridView->getColumns();

	// Distribute column cells.
	int32_t depth = getDepth();
	if (!m_children.empty())
		++depth;

	auto expand = gridView->getBitmap(L"UI.GridView");
	const int32_t size = expand->getSize(gridView).cy;

	Rect rcCell(rect.left, rect.top, rect.left, rect.bottom);
	for (uint32_t i = 0; i < columns.size(); ++i)
	{
		int32_t width = widget->pixel(columns[i]->getWidth());
		if (columns.size() == 1)
			width = rect.getWidth();

		rcCell.right = rcCell.left + width;

		if (i < m_items.size() && m_items[i] != nullptr)
		{
			Rect rcCellLocal = rcCell;
			if (i == 0)
				rcCellLocal.left += depth * size;
			widget->placeCell(m_items[i], rcCellLocal);
		}

		rcCell.left = rcCell.right;
	}

	const Rect rcRow(rect.left, rect.top, std::max(rcCell.right, rect.right), rect.bottom);
	AutoWidgetCell::placeCells(widget, rcRow);
}

void GridRow::interval()
{
	// Cancel pending edit.
	if (m_editMode != 0)
		m_editMode = 0;
}

void GridRow::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	auto expand = getWidget< GridView >()->getBitmap(L"UI.GridView");

	// Handle expand/collapse.
	if (!m_children.empty())
	{
		const int32_t depth = getDepth();
		const int32_t size = expand->getSize(getWidget()).cy;
		const int32_t rx = depth * size + size;
		if (position.x <= rx)
		{
			if (m_state & Expanded)
				m_state &= ~Expanded;
			else
				m_state |= Expanded;

			GridRowStateChangeEvent expandEvent(getWidget< GridView >(), this);
			getWidget< GridView >()->raiseEvent(&expandEvent);
			getWidget< GridView >()->requestUpdate();
			return;
		}
	}

	m_mouseDownPosition = position;

	if (m_editable)
	{
		if (m_editMode == 0)
		{
			// Wait for next tap; cancel wait after 2 seconds.
			getWidget< GridView >()->requestInterval(this, 2000);
			m_editMode = 1;
		}
		else if (m_editMode == 1)
		{
			// Double tap detected; begin edit after mouse is released.
			getWidget< GridView >()->requestInterval(this, 1000);
			m_editMode = 2;
		}
	}

	getWidget< GridView >()->requestUpdate();
}

void GridRow::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	if (m_editMode == 2)
	{
		const int32_t index = getWidget< GridView >()->getColumnIndex(position.x);
		if (index >= 0 && m_items[index] != nullptr)
		{
			const GridColumn* column = getWidget< GridView >()->getColumn(index);
			if (column && column->isEditable())
				getWidget< GridView >()->beginEdit(m_items[index]);
		}
		m_editMode = 0;
	}
}

void GridRow::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
	// Ensure edit isn't triggered.
	m_editMode = 0;
}

void GridRow::mouseMove(MouseMoveEvent* event, const Point& position)
{
	const Size d = position - m_mouseDownPosition;
	if (abs(d.cx) > pixel(2_ut) || abs(d.cy) > pixel(2_ut))
	{
		// Ensure edit isn't triggered if mouse moved during edit state tracking.
		m_editMode = 0;
	}
}

void GridRow::paint(Canvas& canvas, const Rect& rect)
{
	GridView* view = getWidget< GridView >();
	const StyleSheet* ss = getStyleSheet();
	const RefArray< GridColumn >& columns = view->getColumns();
	const int32_t depth = getDepth();
	const bool enabled = view->isEnable(true);

	// Paint custom background.
	if (m_background)
	{
		canvas.setBackground(m_background.resolve(ss));
		canvas.fillRect(rect);
	}
	// No custom background, use different background colors per depth.
	else if (depth > 0)
	{
		if (enabled)
			canvas.setBackground(ss->getColor(this, str(L"background-color-%d", std::min(depth, 4))));
		else
			canvas.setBackground(ss->getColor(this, L"background-color-disabled"));
		canvas.fillRect(rect);
	}

	// Paint selection background.
	if (m_state & GridRow::Selected)
	{
		canvas.setBackground(ss->getColor(this, L"background-color-selected"));
		canvas.fillRect(rect);
	}
	else if (view->getHoverCell() == this)
	{
		canvas.setBackground(ss->getColor(this, L"background-color-hover"));
		canvas.fillRect(rect);
	}

	if (!m_children.empty())
	{
		auto expand = view->getBitmap(L"UI.GridView");
		const int32_t size = expand->getSize(view).cy;
		canvas.drawBitmap(
			Point(rect.left + 2 + depth * size, rect.top + (rect.getHeight() - size) / 2),
			Point((m_state & GridRow::Expanded) ? size : 0, 0),
			Size(size, size),
			expand,
			BlendMode::Alpha
		);
	}

	canvas.setForeground(ss->getColor(this, (m_state & GridRow::Selected) ? L"line-color-selected" : L"line-color"));

	if (columns.size() >= 2)
	{
		int32_t left = rect.left;
		for (auto column : columns)
		{
			left += view->pixel(column->getWidth());
			canvas.drawLine(left, rect.top, left, rect.bottom - 1);
		}
	}

	canvas.setForeground(ss->getColor(this, L"line-color"));
	canvas.drawLine(0, rect.bottom - 1, rect.getWidth(), rect.bottom - 1);
}

}
