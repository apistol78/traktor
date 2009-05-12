#include <stack>
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridDragEvent.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ScrollBar.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Drawing/Image.h"

// Resources
#include "Resources/Expand.h"
#include "Resources/Collapse.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_itemHeight = 18;
const int c_columnsHeight = 25;
const int c_indentWidth = 16;

int getHierarchyDepth(GridRow* row)
{
	int depth = -1;
	while (row)
	{
		++depth;
		row = row->getParent();
	}
	return depth;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridView", GridView, Widget)

bool GridView::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_expand[0] = Bitmap::load(c_ResourceExpand, sizeof(c_ResourceExpand), L"png");
	m_expand[1] = Bitmap::load(c_ResourceCollapse, sizeof(c_ResourceCollapse), L"png");

	m_scrollBarRows = gc_new< ScrollBar >();
	m_scrollBarRows->create(this, ScrollBar::WsVertical);
	m_scrollBarRows->addScrollEventHandler(createMethodHandler(this, &GridView::eventScroll));

	addButtonDownEventHandler(createMethodHandler(this, &GridView::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &GridView::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &GridView::eventMouseMove));
	addMouseWheelEventHandler(createMethodHandler(this, &GridView::eventMouseWheel));
	addPaintEventHandler(createMethodHandler(this, &GridView::eventPaint));
	addSizeEventHandler(createMethodHandler(this, &GridView::eventSize));

	m_dragActive = false;
	m_dragEnabled = bool((style & WsDrag) == WsDrag);
	m_columnHeader = bool((style & WsColumnHeader) == WsColumnHeader);
	m_lastSelected = -1;

	return true;
}

void GridView::addColumn(GridColumn* column)
{
	m_columns.push_back(column);
}

void GridView::addRow(GridRow* row)
{
	m_rows.push_back(row);
}

void GridView::removeAllRows()
{
	m_rows.resize(0);
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

	return int(outRows.size());
}

void GridView::addSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

void GridView::addDragEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiDrag, eventHandler);
}

void GridView::addDragValidEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiDragValid, eventHandler);
}

Size GridView::getPreferedSize() const
{
	int width = 0;
	for (RefArray< GridColumn >::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
		width += (*i)->getWidth();
	return Size(width, 256);
}

void GridView::update(const Rect* rc, bool immediate)
{
	updateScrollBars();
	Widget::update(rc, immediate);
}

void GridView::updateScrollBars()
{
	Rect rc = getInnerRect();

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

	int rowCount = int(rows.size());
	int pageCount = 1 + (rc.getHeight() - (m_columnHeader ? c_columnsHeight : 0)) / c_itemHeight;

	m_scrollBarRows->setRange(rowCount);
	m_scrollBarRows->setPage(pageCount);
	m_scrollBarRows->setVisible(rowCount >= pageCount);
	m_scrollBarRows->update();
}

int GridView::dropRow() const
{
	return (m_dragPosition.y - (m_columnHeader ? c_columnsHeight : 0) + c_itemHeight / 2) / c_itemHeight;
}

bool GridView::dropValid()
{
	int row = dropRow();

	if (row < 0)
		return false;

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

	if (row > int(rows.size()))
		return false;

	// Cannot drag into own children.
	Ref< GridRow > sibling = rows[std::max(row - 1, 0)];
	for (Ref< GridRow > parent = sibling->getParent(); parent; parent = parent->getParent())
	{
		if (parent == m_dragRow)
			return false;
	}

	// Check with user if drop still valid.
	GridDragEvent dragEvent(this, m_dragRow, row);
	raiseEvent(EiDragValid, &dragEvent);
	if (dragEvent.cancelled())
		return false;

	return true;
}

void GridView::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);

	if (mouseEvent->getButton() != MouseEvent::BtLeft)
		return;

	Point pos = mouseEvent->getPosition();
	Rect rc = getInnerRect();

	if (m_columnHeader && pos.y <= c_columnsHeight)
	{
		int x = 0;
		for (RefArray< GridColumn >::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
		{
			int cx = x + (*i)->getWidth();
			if (pos.x - 3 < cx && pos.x + 3 > cx)
			{
				m_resizeColumn = *i;
				m_resizeColumnLeft = x;
				setCapture();
				setCursor(CrSizeWE);
				break;
			}
			x = cx;
		}
	}
	else
	{
		// Row click.

		bool modifier = bool((mouseEvent->getKeyState() & (KsShift | KsControl)) != 0);

		// Deselect all rows if no modifier isn't being held.
		if (!modifier)
		{
			RefArray< GridRow > rows;
			getRows(rows, GfDescendants);
			for (RefArray< GridRow >::iterator i = rows.begin(); i != rows.end(); ++i)
				(*i)->setState((*i)->getState() & ~GridRow::RsSelected);
			m_lastSelected = -1;
		}

		RefArray< GridRow > rows;
		getRows(rows, GfDescendants | GfExpandedOnly);

		int row = m_scrollBarRows->getPosition();
		int index = row + (pos.y - (m_columnHeader ? c_columnsHeight : 0)) / c_itemHeight;
		if (index >= 0 && index < int(rows.size()))
		{
			uint32_t state = rows[index]->getState();

			// Expand or collapse row.
			int depth = getHierarchyDepth(rows[index]);
			if (!modifier && !rows[index]->getChildren().empty() && pos.x >= 2 + depth * c_indentWidth && pos.x <= 2 + depth * c_indentWidth + m_expand[0]->getSize().cx)
			{
				if (state & GridRow::RsExpanded)
					state &= ~GridRow::RsExpanded;
				else
					state |= GridRow::RsExpanded;

				m_lastSelected = -1;
			}
			else
			{
				if (m_lastSelected >= 0 && (mouseEvent->getKeyState() & KsShift) == KsShift)
				{
					int from = index;
					int to = m_lastSelected;

					if (from > to)
						std::swap(from, to);

					for (int i = from; i <= to; ++i)
					{
						int state = rows[i]->getState();
						rows[i]->setState(state | GridRow::RsSelected);
					}
				
					state |= GridRow::RsSelected;
					m_lastSelected = index;
				}
				else
				{
					if (state & GridRow::RsSelected)
					{
						state &= ~GridRow::RsSelected;
						m_lastSelected = -1;
					}
					else
					{
						state |= GridRow::RsSelected;
						m_lastSelected = index;

						// Remember which item selected to determine if we should enter "drag" mode.
						if (m_dragEnabled && !modifier)
						{
							m_dragRow = rows[index];
							m_dragActive = false;
							setCapture();
						}
					}
				}
			}

			rows[index]->setState(state);
		}

		updateScrollBars();
		update();

		raiseEvent(EiSelectionChange, 0);
	}
}

void GridView::eventButtonUp(Event* event)
{
	if (m_resizeColumn)
	{
		m_resizeColumn = 0;
		releaseCapture();
		update();
	}
	else if (m_dragEnabled && m_dragRow)
	{
		if (m_dragActive)
		{
			//RefArray< GridRow > rows;
			//getRows(rows, GfDescendants | GfExpandedOnly);
			//T_ASSERT (!rows.empty());

			//int dragTargetRow = (m_dragPosition.y - (m_columnHeader ? c_columnsHeight : 0) + c_itemHeight / 2) / c_itemHeight - 1;

			//dragTargetRow = std::max(dragTargetRow, 0);
			//dragTargetRow = std::min(dragTargetRow, int(rows.size()));

			//GridDragEvent dragEvent(this, m_dragRow, dragTargetRow);
			//raiseEvent(EiDrag, &dragEvent);
			//if (!dragEvent.cancelled())
			//{
			//	m_dragRow->getParent()->removeChild(m_dragRow);

			//	if (dragTargetRow > 0)
			//	{
			//		Ref< GridRow > targetRow = rows[dragTargetRow - 1];
			//		if (targetRow->getParent())
			//		{
			//			targetRow->getParent()->insertChildAfter(targetRow, m_dragRow);
			//		}
			//		else
			//		{
			//			RefArray< GridRow >::iterator i = std::find(m_rows.begin(), m_rows.end(), targetRow);
			//			m_rows.insert(i, m_dragRow);
			//		}
			//	}
			//	else
			//		m_rows.insert(m_rows.begin(), m_dragRow);
			//}
		}

		m_dragRow = 0;
		m_dragActive = false;

		releaseCapture();
		update();
	}
}

void GridView::eventMouseMove(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	Point pos = mouseEvent->getPosition();

	if (m_resizeColumn)
	{
		int width = std::max(0, pos.x - m_resizeColumnLeft);
		m_resizeColumn->setWidth(width);

		setCursor(CrSizeWE);

		update();
		event->consume();
	}
	else if (m_dragEnabled && m_dragRow)
	{
		setCursor(CrHand);

		m_dragPosition = pos;
		m_dragActive = true;

		update();
		event->consume();
	}
	else if (m_columnHeader && pos.y <= c_columnsHeight)
	{
		int x = 0;
		for (RefArray< GridColumn >::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
		{
			int cx = x + (*i)->getWidth();
			if (pos.x - 3 < cx && pos.x + 3 > cx)
			{
				setCursor(CrSizeWE);
				break;
			}
			x = cx;
		}
		event->consume();
	}
}

void GridView::eventMouseWheel(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);

	int position = m_scrollBarRows->getPosition();
	position -= mouseEvent->getWheelRotation() * 4;
	m_scrollBarRows->setPosition(position);

	update();
}

void GridView::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Rect rc = getInnerRect();

	canvas.setBackground(getSystemColor(ScWindowBackground));
	canvas.fillRect(rc);

	// Draw columns.
	if (m_columnHeader)
	{
		canvas.setForeground(Color(255, 255, 255));
		canvas.setBackground(getSystemColor(ScButtonFace));
		canvas.fillGradientRect(Rect(rc.left, rc.top, rc.right, rc.top + c_columnsHeight));

		int left = 0;

		for (uint32_t i = 0; i < m_columns.size(); ++i)
		{
			GridColumn* column = m_columns[i];

			canvas.setForeground(getSystemColor(ScWindowText));
			canvas.drawText(Rect(left + 2, rc.top, left + column->getWidth() - 2, rc.top + c_columnsHeight), column->getTitle(), AnLeft, AnCenter);

			if (i > 0)
			{
				canvas.setForeground(Color(208, 208, 208));
				canvas.drawLine(left, rc.top + 4, left, rc.top + c_columnsHeight - 4);
			}

			left += column->getWidth();
		}
	}

	if (m_columns.empty())
		return;

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

	// Draw rows.
	{
		int top = m_columnHeader ? c_columnsHeight - 1 : -1;
		int row = m_scrollBarRows->getPosition();
		int page = m_scrollBarRows->getPage();

		Font defaultFont = getFont();

		for (int i = 0; i < page; ++i)
		{
			if (row + i >= int(rows.size()))
				break;

			GridRow* rw = rows[row + i];

			if (rw->getFont())
				canvas.setFont(*rw->getFont());

			int depth = getHierarchyDepth(rw);

			if (rw->getState() & GridRow::RsSelected)
			{
				canvas.setForeground(Color(240, 240, 250));
				canvas.setBackground(Color(220, 220, 230));
				canvas.fillGradientRect(Rect(rc.left, rc.top + top + 1, rc.right, rc.top + top + c_itemHeight));
			}

			if (!rw->getChildren().empty())
			{
				Bitmap* expand = m_expand[(rw->getState() & GridRow::RsExpanded) ? 1 : 0];
				canvas.drawBitmap(
					Point(2 + depth * c_indentWidth, top + (c_itemHeight - expand->getSize().cy) / 2),
					Point(0, 0),
					expand->getSize(),
					expand
				);
			}

			int left = m_columns[0]->getWidth();

			const RefArray< GridItem >& items = rw->getItems();

			canvas.setForeground(getSystemColor(ScWindowText));
			canvas.drawText(Rect(16 + depth * c_indentWidth, top, left - 2, top + c_itemHeight), items[0]->getText(), AnLeft, AnCenter);

			for (uint32_t j = 1; j < items.size(); ++j)
			{
				if (j >= m_columns.size())
					break;

				canvas.setForeground(Color(190, 190, 200));
				canvas.drawLine(left, top, left, top + c_itemHeight);

				canvas.setForeground(getSystemColor(ScWindowText));
				canvas.drawText(Rect(left + 2, top, left + m_columns[j]->getWidth() - 2, top + c_itemHeight), items[j]->getText(), AnLeft, AnCenter);

				left += m_columns[j]->getWidth();
			}

			canvas.setForeground(Color(190, 190, 200));
			canvas.drawLine(rc.left, top + c_itemHeight, rc.right, top + c_itemHeight);

			if (rw->getFont())
				canvas.setFont(defaultFont);

			top += c_itemHeight;
		}
	}

	// Draw drag image.
	if (m_dragEnabled && m_dragActive && dropValid())
	{
		int row = dropRow();
		int line = row * c_itemHeight + (m_columnHeader ? c_columnsHeight : 0);

		canvas.setForeground(Color(0, 0, 0));
		canvas.drawLine(rc.left, line, rc.right, line);

		canvas.setForeground(Color(128, 128, 128));
		canvas.drawLine(rc.left, line - 1, rc.right, line - 1);
		canvas.drawLine(rc.left, line + 1, rc.right, line + 1);
	}

	event->consume();
}

void GridView::eventSize(Event* event)
{
	int width = m_scrollBarRows->getPreferedSize().cx;

	Rect inner = getInnerRect();
	Rect rc(
		Point(inner.getWidth() - width, (m_columnHeader ? c_columnsHeight : 0)),
		Size(width, inner.getHeight() - (m_columnHeader ? c_columnsHeight : 0))
	);

	m_scrollBarRows->setRect(rc);

	updateScrollBars();
}

void GridView::eventScroll(Event* event)
{
	update();
}

		}
	}
}
