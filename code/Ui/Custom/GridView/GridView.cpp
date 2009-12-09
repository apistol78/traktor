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

const int32_t c_itemHeight = 18;
const int32_t c_columnsHeight = 25;
const int32_t c_indentWidth = 16;
const int32_t c_imageMargin = 4;
const int32_t c_imageSize = 12;

int32_t getHierarchyDepth(GridRow* row)
{
	int32_t depth = -1;
	while (row)
	{
		++depth;
		row = row->getParent();
	}
	return depth;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridView", GridView, Widget)

GridView::GridView()
:	m_imageWidth(c_imageSize)
,	m_imageHeight(c_imageSize)
,	m_imageCount(0)
{
}

bool GridView::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_expand[0] = Bitmap::load(c_ResourceExpand, sizeof(c_ResourceExpand), L"png");
	m_expand[1] = Bitmap::load(c_ResourceCollapse, sizeof(c_ResourceCollapse), L"png");

	m_scrollBarRows = new ScrollBar();
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

uint32_t GridView::addImage(Bitmap* image, uint32_t imageCount)
{
	T_ASSERT (image);
	T_ASSERT (imageCount > 0);

	if (m_image)
	{
		Ref< Bitmap > source = image;

		uint32_t width = m_image->getSize().cx + source->getSize().cx;
		uint32_t height = std::max(m_image->getSize().cy, source->getSize().cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_image->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), source->getSize()), Point(m_image->getSize().cx, 0));
		m_image = newImage;
	}
	else
	{
		m_image = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_image->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_image->getSize().cy);
	}

	uint32_t imageBase = m_imageCount;
	m_imageCount += imageCount;

	return imageBase;
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
	int32_t width = 0;
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

	int32_t rowCount = int32_t(rows.size());
	int32_t pageCount = 1 + (rc.getHeight() - (m_columnHeader ? c_columnsHeight : 0)) / c_itemHeight;

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
	int32_t row = dropRow();

	if (row < 0)
		return false;

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

	if (row > int32_t(rows.size()))
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
		if (m_columns.size() > 1)
		{
			int32_t x = 0;
			for (RefArray< GridColumn >::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
			{
				int32_t cx = x + (*i)->getWidth();
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

		int32_t row = m_scrollBarRows->getPosition();
		int32_t index = row + (pos.y - (m_columnHeader ? c_columnsHeight : 0)) / c_itemHeight;
		if (index >= 0 && index < int32_t(rows.size()))
		{
			uint32_t state = rows[index]->getState();

			// Expand or collapse row.
			int32_t depth = getHierarchyDepth(rows[index]);
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
					int32_t from = index;
					int32_t to = m_lastSelected;

					if (from > to)
						std::swap(from, to);

					for (int32_t i = from; i <= to; ++i)
					{
						int32_t state = rows[i]->getState();
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

			//int32_t dragTargetRow = (m_dragPosition.y - (m_columnHeader ? c_columnsHeight : 0) + c_itemHeight / 2) / c_itemHeight - 1;

			//dragTargetRow = std::max(dragTargetRow, 0);
			//dragTargetRow = std::min(dragTargetRow, int32_t(rows.size()));

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
		int32_t width = std::max(0, pos.x - m_resizeColumnLeft);
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
		if (m_columns.size() > 1)
		{
			int32_t x = 0;
			for (RefArray< GridColumn >::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i)
			{
				int32_t cx = x + (*i)->getWidth();
				if (pos.x - 3 < cx && pos.x + 3 > cx)
				{
					setCursor(CrSizeWE);
					break;
				}
				x = cx;
			}
		}
		event->consume();
	}
}

void GridView::eventMouseWheel(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);

	int32_t position = m_scrollBarRows->getPosition();
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

		int32_t left = 0;

		for (uint32_t i = 0; i < m_columns.size(); ++i)
		{
			GridColumn* column = m_columns[i];

			int32_t width = column->getWidth();
			if (m_columns.size() == 1)
				width = rc.getWidth();

			canvas.setForeground(getSystemColor(ScWindowText));
			canvas.drawText(Rect(left + 2, rc.top, left + width - 2, rc.top + c_columnsHeight), column->getTitle(), AnLeft, AnCenter);

			if (i > 0)
			{
				canvas.setForeground(Color(208, 208, 208));
				canvas.drawLine(left, rc.top + 4, left, rc.top + c_columnsHeight - 4);
			}

			left += width;
		}
	}

	if (m_columns.empty())
		return;

	RefArray< GridRow > rows;
	getRows(rows, GfDescendants | GfExpandedOnly);

	// Draw rows.
	{
		int32_t top = m_columnHeader ? c_columnsHeight - 1 : -1;
		int32_t row = m_scrollBarRows->getPosition();
		int32_t page = m_scrollBarRows->getPage();

		Font defaultFont = getFont();

		for (int32_t i = 0; i < page; ++i)
		{
			if (row + i >= int32_t(rows.size()))
				break;

			Ref< GridRow > rw = rows[row + i];

			if (rw->getFont())
				canvas.setFont(*rw->getFont());

			int32_t depth = getHierarchyDepth(rw);

			if (rw->getState() & GridRow::RsSelected)
			{
				canvas.setForeground(Color(240, 240, 250));
				canvas.setBackground(Color(220, 220, 230));
				canvas.fillGradientRect(Rect(rc.left, rc.top + top + 1, rc.right, rc.top + top + c_itemHeight));
			}

			if (!rw->getChildren().empty())
			{
				Ref< Bitmap > expand = m_expand[(rw->getState() & GridRow::RsExpanded) ? 1 : 0];
				canvas.drawBitmap(
					Point(2 + depth * c_indentWidth, top + (c_itemHeight - expand->getSize().cy) / 2),
					Point(0, 0),
					expand->getSize(),
					expand
				);
			}

			const RefArray< GridItem >& items = rw->getItems();

			int32_t left = 16 + depth * c_indentWidth;

			int32_t imageIndex = (rw->getState() & GridRow::RsExpanded) ? items[0]->getExpandedImage() : items[0]->getImage();
			if (imageIndex >= 0)
			{
				int32_t centerOffsetY = (c_itemHeight - m_imageHeight) / 2;
				canvas.drawBitmap(
					Point(left, top + centerOffsetY),
					Point(imageIndex * m_imageWidth, 0),
					Size(m_imageWidth, m_imageHeight),
					m_image,
					BmAlpha
				);
				left += m_imageWidth + c_imageMargin;
			}

			int32_t width = (m_columns.size() > 1) ? m_columns[0]->getWidth() : rc.getWidth();

			canvas.setForeground(getSystemColor(ScWindowText));
			canvas.drawText(Rect(left, top, width - 2, top + c_itemHeight), items[0]->getText(), AnLeft, AnCenter);

			if (m_columns.size() > 1)
			{
				left = m_columns[0]->getWidth();

				for (uint32_t j = 1; j < items.size(); ++j)
				{
					if (j >= m_columns.size())
						break;

					int32_t width = m_columns[j]->getWidth();

					canvas.setForeground(Color(190, 190, 200));
					canvas.drawLine(left, top, left, top + c_itemHeight);

					canvas.setForeground(getSystemColor(ScWindowText));
					canvas.drawText(Rect(left + 2, top, left + width - 2, top + c_itemHeight), items[j]->getText(), AnLeft, AnCenter);

					left += width;
				}
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
		int32_t row = dropRow();
		int32_t line = row * c_itemHeight + (m_columnHeader ? c_columnsHeight : 0);

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
	int32_t width = m_scrollBarRows->getPreferedSize().cx;

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
