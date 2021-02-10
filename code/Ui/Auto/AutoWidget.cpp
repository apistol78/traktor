#include <limits>
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/ScrollBar.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutoWidget", AutoWidget, Widget)

AutoWidget::AutoWidget()
:	m_scrollOffset(0, 0)
,	m_deferredUpdate(false)
{
}

bool AutoWidget::create(ui::Widget* parent, int32_t style)
{
	if (!ui::Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &AutoWidget::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &AutoWidget::eventButtonUp);
	addEventHandler< MouseDoubleClickEvent >(this, &AutoWidget::eventDoubleClick);
	addEventHandler< MouseMoveEvent >(this, &AutoWidget::eventMouseMove);
	addEventHandler< MouseWheelEvent >(this, &AutoWidget::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &AutoWidget::eventPaint);
	addEventHandler< SizeEvent >(this, &AutoWidget::eventSize);
	addEventHandler< TimerEvent >(this, &AutoWidget::eventTimer);

	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarV = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBarH->addEventHandler< ScrollEvent >(this, &AutoWidget::eventScroll);
	m_scrollBarV->addEventHandler< ScrollEvent >(this, &AutoWidget::eventScroll);

	startTimer(100);
	return true;
}

void AutoWidget::setFocusCell(AutoWidgetCell* focusCell)
{
	m_focusCell = focusCell;
}

AutoWidgetCell* AutoWidget::getFocusCell() const
{
	return m_focusCell;
}

AutoWidgetCell* AutoWidget::hitTest(const Point& position)
{
	AutoWidgetCell* hit = nullptr;

	if (m_headerCell.cell)
	{
		if (m_headerCell.rc.inside(position))
			return m_headerCell.cell;
	}

	if (m_footerCell.cell)
	{
		if (m_footerCell.rc.inside(position))
			return m_footerCell.cell;
	}

	Point clientPosition = position - m_scrollOffset;
	for (std::vector< CellInstance >::reverse_iterator i = m_cells.rbegin(); i != m_cells.rend(); ++i)
	{
		if (!i->rc.inside(clientPosition))
			continue;

		if ((hit = i->cell->hitTest(clientPosition)) != nullptr)
			break;
	}

	return hit;
}

void AutoWidget::requestUpdate()
{
	m_deferredUpdate = true;
}

void AutoWidget::requestInterval(AutoWidgetCell* cell, int32_t duration)
{
	// Update already enqueued interval first.
	for (auto& interval : m_intervals)
	{
		if (interval.cell == cell)
		{
			interval.duration = duration;
			return;
		}
	}

	// Not queued, then add new entry.
	CellInterval ci;
	ci.cell = cell;
	ci.duration = duration;
	m_intervals.push_back(ci);
}

void AutoWidget::placeCell(AutoWidgetCell* cell, const Rect& rect)
{
	T_ASSERT(cell);

	// Add this cell instance.
	CellInstance instance = { cell, rect };
	m_cells.push_back(instance);

	// Notify cell to place sub-cells.
	cell->placeCells(this, rect);
}

void AutoWidget::placeHeaderCell(AutoWidgetCell* cell, int32_t height)
{
	T_ASSERT(cell);

	ui::Rect inner = getInnerRect();
	inner.bottom = inner.top + height;

	m_headerCell.cell = cell;
	m_headerCell.rc = inner;

	cell->placeCells(this, inner);
}

void AutoWidget::placeFooterCell(AutoWidgetCell* cell, int32_t height)
{
	T_ASSERT(cell);

	ui::Rect inner = getInnerRect();
	inner.bottom = inner.top + height;

	m_footerCell.cell = cell;
	m_footerCell.rc = inner;

	cell->placeCells(this, inner);
}

Rect AutoWidget::getCellRect(const AutoWidgetCell* cell) const
{
	if (cell)
	{
		if (m_headerCell.cell == cell)
			return m_headerCell.rc;

		if (m_footerCell.cell == cell)
			return m_footerCell.rc;

		for (const auto& instance : m_cells)
		{
			if (instance.cell == cell)
				return instance.rc;
		}
	}
	return Rect();
}

Rect AutoWidget::getCellClientRect(const AutoWidgetCell* cell) const
{
	if (cell)
	{
		if (m_headerCell.cell == cell)
			return m_headerCell.rc;

		if (m_footerCell.cell == cell)
			return m_footerCell.rc;

		for (const auto& instance : m_cells)
		{
			if (instance.cell == cell)
				return instance.rc.offset(m_scrollOffset);
		}
	}
	return Rect();
}

bool AutoWidget::setCapturedCell(AutoWidgetCell* cell)
{
	releaseCapturedCell();

	if (!cell->beginCapture())
		return false;

	setCapture();

	m_captureCell = cell;
	return true;
}

void AutoWidget::releaseCapturedCell()
{
	if (m_captureCell)
	{
		m_captureCell->endCapture();
		releaseCapture();
	}
	m_captureCell = nullptr;
}

const Size& AutoWidget::getScrollOffset() const
{
	return m_scrollOffset;
}

Point AutoWidget::getClientPosition(const Point& innerPosition) const
{
	return innerPosition - m_scrollOffset;
}

void AutoWidget::updateLayout()
{
	// Remove references to previously layed out cells.
	m_headerCell.cell = nullptr;
	m_footerCell.cell = nullptr;
	m_cells.resize(0);

	Rect innerRect = getInnerRect();

	bool scrollBarVisibleH = m_scrollBarH->isVisible(false);
	bool scrollBarVisibleV = m_scrollBarV->isVisible(false);

	if (scrollBarVisibleH)
		innerRect.bottom -= m_scrollBarH->getPreferedSize().cy;
	if (scrollBarVisibleV)
		innerRect.right -= m_scrollBarV->getPreferedSize().cx;

	if (innerRect.getWidth() <= 0 || innerRect.getHeight() <= 0)
		return;

	layoutCells(innerRect);

	// Calculate bounds from placed cells.
	m_bounds.left = std::numeric_limits< int32_t >::max();
	m_bounds.right = -std::numeric_limits< int32_t >::max();
	m_bounds.top = std::numeric_limits< int32_t >::max();
	m_bounds.bottom = -std::numeric_limits< int32_t >::max();

	for (const auto& instance : m_cells)
	{
		m_bounds.left = std::min(m_bounds.left, instance.rc.left);
		m_bounds.right = std::max(m_bounds.right, instance.rc.right);
		m_bounds.top = std::min(m_bounds.top, instance.rc.top);
		m_bounds.bottom = std::max(m_bounds.bottom, instance.rc.bottom);
	}

	// Update scrollbar ranges.
	int32_t columnCount = (m_bounds.right + 15) / 16;
	int32_t columnPageCount = (innerRect.right + 15) / 16;
	int32_t rowCount = (m_bounds.bottom + 15) / 16;
	int32_t rowPageCount = (innerRect.bottom + 15) / 16;

	m_scrollBarH->setRange(columnCount);
	m_scrollBarH->setPage(columnPageCount);
	m_scrollBarH->setVisible(columnCount > columnPageCount);
	m_scrollBarH->update();

	m_scrollBarV->setRange(rowCount);
	m_scrollBarV->setPage(rowPageCount);
	m_scrollBarV->setVisible(rowCount > rowPageCount);
	m_scrollBarV->update();

	// Need to re-layout cells in case scroll bar visibility state has changed.
	if (
		m_scrollBarH->isVisible(false) != scrollBarVisibleH ||
		m_scrollBarV->isVisible(false) != scrollBarVisibleV
	)
	{
		placeScrollBars();

		Rect innerRect = getInnerRect();

		if (m_scrollBarH->isVisible(false))
			innerRect.bottom -= m_scrollBarH->getPreferedSize().cy;
		if (m_scrollBarV->isVisible(false))
			innerRect.right -= m_scrollBarV->getPreferedSize().cx;

		m_headerCell.cell = nullptr;
		m_footerCell.cell = nullptr;
		m_cells.resize(0);

		layoutCells(innerRect);
	}

	if (!m_scrollBarH->isVisible(false))
		m_scrollOffset.cx = 0;
	if (!m_scrollBarV->isVisible(false))
		m_scrollOffset.cy = 0;
}

void AutoWidget::placeScrollBars()
{
	int32_t width = m_scrollBarV->getPreferedSize().cx;
	int32_t height = m_scrollBarH->getPreferedSize().cy;

	Rect innerRect = getInnerRect();

	if (m_headerCell.cell)
		innerRect.top = m_headerCell.rc.bottom + 1;
	if (m_footerCell.cell)
		innerRect.bottom = m_footerCell.rc.top - 1;

	int32_t widthH = innerRect.getWidth();
	if (m_scrollBarV->isVisible(false))
		widthH -= width;

	int32_t heightV = innerRect.getHeight();
	if (m_scrollBarH->isVisible(false))
		heightV -= height;

	m_scrollBarH->setRect(Rect(
		Point(0, innerRect.top + innerRect.getHeight() - height),
		Size(widthH, height)
	));
	m_scrollBarV->setRect(Rect(
		Point(innerRect.getWidth() - width, innerRect.top),
		Size(width, heightV)
	));
}

void AutoWidget::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	Ref< AutoWidgetCell > hitItem = hitTest(event->getPosition());
	if (hitItem)
	{
		m_focusCell = hitItem;
		if (setCapturedCell(hitItem))
		{
			Point clientPosition = event->getPosition() - m_scrollOffset;
			m_captureCell->mouseDown(event, clientPosition);
		}
	}
	else
		m_focusCell = nullptr;

	update();
}

void AutoWidget::eventButtonUp(MouseButtonUpEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	if (m_captureCell)
	{
		Point clientPosition = event->getPosition() - m_scrollOffset;
		m_captureCell->mouseUp(event, clientPosition);
	}

	releaseCapturedCell();
	update();
}

void AutoWidget::eventDoubleClick(MouseDoubleClickEvent* event)
{
	Ref< AutoWidgetCell > hitItem = hitTest(event->getPosition());
	if (hitItem)
	{
		Point clientPosition = event->getPosition() - m_scrollOffset;
		hitItem->mouseDoubleClick(event, clientPosition);
	}
}

void AutoWidget::eventMouseMove(MouseMoveEvent* event)
{
	if (m_captureCell)
	{
		Point clientPosition = event->getPosition() - m_scrollOffset;
		m_captureCell->mouseMove(event, clientPosition);
	}
	if (m_focusCell)
	{
		Point clientPosition = event->getPosition() - m_scrollOffset;
		m_focusCell->mouseMoveFocus(event, clientPosition);
	}
	if (m_captureCell || m_focusCell)
		update();
}

void AutoWidget::eventMouseWheel(MouseWheelEvent* event)
{
	// Calculate new position.
	int32_t position = m_scrollBarV->getPosition();
	position -= event->getRotation() * 4;

	// Set scrollbar position.
	m_scrollBarV->setPosition(position);
	m_scrollOffset.cy = -m_scrollBarV->getPosition() * 16;

	// Ensure scroll events are issued.
	ScrollEvent scrollEvent(this, 0);
	raiseEvent(&scrollEvent);

	// Redraw widget.
	update();
}

void AutoWidget::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect innerRect = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(innerRect);

	for (const auto& instance : m_cells)
	{
		Rect rc = instance.rc.offset(m_scrollOffset);
		if (rc.intersect(innerRect))
			instance.cell->paint(canvas, rc);
	}

	if (m_headerCell.cell)
	{
		Rect rc = m_headerCell.rc.offset(m_scrollOffset.cx, 0);
		m_headerCell.cell->paint(canvas, rc);
	}

	if (m_footerCell.cell)
	{
		Rect rc = m_footerCell.rc.offset(m_scrollOffset.cx, 0);
		m_footerCell.cell->paint(canvas, rc);
	}

	event->consume();
}

void AutoWidget::eventSize(SizeEvent* event)
{
	updateLayout();
	placeScrollBars();
	update();
}

void AutoWidget::eventTimer(TimerEvent* event)
{
	for (std::list< CellInterval >::iterator i = m_intervals.begin(); i != m_intervals.end(); )
	{
		if ((i->duration -= 100) <= 0)
		{
			i->cell->interval();
			i = m_intervals.erase(i);
		}
		else
			++i;
	}

	if (!isVisible(false))
		return;

	if (m_deferredUpdate)
	{
		m_deferredUpdate = false;
		updateLayout();
		update();
	}
}

void AutoWidget::eventScroll(ScrollEvent* event)
{
	m_scrollOffset.cx = -m_scrollBarH->getPosition() * 16;
	m_scrollOffset.cy = -m_scrollBarV->getPosition() * 16;

	ScrollEvent scrollEvent(this, 0);
	raiseEvent(&scrollEvent);

	update();
}

	}
}
