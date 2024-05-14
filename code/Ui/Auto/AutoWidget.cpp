/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/ScrollBar.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutoWidget", AutoWidget, Widget)

bool AutoWidget::create(ui::Widget* parent, uint32_t style)
{
	if (!ui::Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &AutoWidget::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &AutoWidget::eventButtonUp);
	addEventHandler< MouseDoubleClickEvent >(this, &AutoWidget::eventDoubleClick);
	addEventHandler< MouseMoveEvent >(this, &AutoWidget::eventMouseMove);
	addEventHandler< MouseWheelEvent >(this, &AutoWidget::eventMouseWheel);
	addEventHandler< MouseTrackEvent >(this, &AutoWidget::eventMouseTrack);
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

	if (m_headerCell)
	{
		if (m_headerCell->getRect().inside(position))
			return m_headerCell;
	}

	if (m_footerCell)
	{
		if (m_footerCell->getRect().inside(position))
			return m_footerCell;
	}

	const Point clientPosition = position - m_scrollOffset;
	for (int32_t i = (int32_t)m_cells.size() - 1; i >= 0; --i)
	{
		AutoWidgetCell* cell = m_cells[i];
		if (!cell->getRect().inside(clientPosition))
			continue;

		if ((hit = cell->hitTest(clientPosition)) != nullptr)
			break;
	}

	return hit;
}

AutoWidgetCell* AutoWidget::getHoverCell() const
{
	return m_hoverCell;
}

AutoWidgetCell* AutoWidget::getHeaderCell() const
{
	return m_headerCell;
}

AutoWidgetCell* AutoWidget::getFooterCell() const
{
	return m_footerCell;
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
	m_intervals.push_back({ cell, duration });
}

void AutoWidget::placeCell(AutoWidgetCell* cell, const Rect& rect)
{
	T_ASSERT(cell);

	// Add this cell instance.
	m_cells.push_back(cell);

	// Notify cell to place sub-cells.
	cell->placeCells(this, rect);
}

void AutoWidget::placeHeaderCell(AutoWidgetCell* cell, int32_t height)
{
	T_ASSERT(cell);

	ui::Rect inner = getInnerRect();
	inner.bottom = inner.top + height;

	m_headerCell = cell;

	cell->placeCells(this, inner);
}

void AutoWidget::placeFooterCell(AutoWidgetCell* cell, int32_t height)
{
	T_ASSERT(cell);

	ui::Rect inner = getInnerRect();
	inner.bottom = inner.top + height;

	m_footerCell = cell;

	cell->placeCells(this, inner);
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

void AutoWidget::scrollTo(const Point& pnt)
{
	const Rect innerRect = getInnerRect();

	m_scrollOffset = -(pnt - innerRect.getCenter());

	if (m_scrollBarH->isVisible(false))
		m_scrollBarH->setPosition(-m_scrollOffset.cx / 16);
	else
		m_scrollOffset.cx = 0;

	if (m_scrollBarV->isVisible(false))
		m_scrollBarV->setPosition(-m_scrollOffset.cy / 16);
	else
		m_scrollOffset.cy = 0;

	update();
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
	m_headerCell = nullptr;
	m_footerCell = nullptr;
	m_cells.resize(0);

	Rect innerRect = getInnerRect();

	const bool scrollBarVisibleH = m_scrollBarH->isVisible(false);
	const bool scrollBarVisibleV = m_scrollBarV->isVisible(false);

	if (scrollBarVisibleH)
		innerRect.bottom -= m_scrollBarH->getPreferredSize(innerRect.getSize()).cy;
	if (scrollBarVisibleV)
		innerRect.right -= m_scrollBarV->getPreferredSize(innerRect.getSize()).cx;

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
		const Rect rc = instance->getRect();
		m_bounds.left = std::min(m_bounds.left, rc.left);
		m_bounds.right = std::max(m_bounds.right, rc.right);
		m_bounds.top = std::min(m_bounds.top, rc.top);
		m_bounds.bottom = std::max(m_bounds.bottom, rc.bottom);
	}

	// Update scrollbar ranges.
	const int32_t columnCount = (m_bounds.right + 15) / 16;
	const int32_t columnPageCount = (innerRect.right + 15) / 16;
	const int32_t rowCount = (m_bounds.bottom + 15) / 16;
	const int32_t rowPageCount = (innerRect.bottom + 15) / 16;

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
			innerRect.bottom -= m_scrollBarH->getPreferredSize(innerRect.getSize()).cy;
		if (m_scrollBarV->isVisible(false))
			innerRect.right -= m_scrollBarV->getPreferredSize(innerRect.getSize()).cx;

		m_headerCell = nullptr;
		m_footerCell = nullptr;
		m_cells.resize(0);

		layoutCells(innerRect);
	}

	if (!m_scrollBarH->isVisible(false))
	{
		m_scrollBarH->setPosition(0);
		m_scrollOffset.cx = 0;
	}
	if (!m_scrollBarV->isVisible(false))
	{
		m_scrollBarV->setPosition(0);
		m_scrollOffset.cy = 0;
	}

	// Update hover tracking.
	Ref< AutoWidgetCell > hitItem = hitTest(getMousePosition());
	if (m_hoverCell != hitItem)
	{
		if (m_hoverCell != nullptr)
			m_hoverCell->mouseLeave();
		if ((m_hoverCell = hitItem) != nullptr)
			m_hoverCell->mouseEnter();
	}
}

void AutoWidget::placeScrollBars()
{
	Rect innerRect = getInnerRect();

	const int32_t width = m_scrollBarV->getPreferredSize(innerRect.getSize()).cx;
	const int32_t height = m_scrollBarH->getPreferredSize(innerRect.getSize()).cy;

	if (m_headerCell)
	{
		const Rect rc = m_headerCell->getRect();
		innerRect.top = rc.bottom + 1;
	}
	if (m_footerCell)
	{
		const Rect rc = m_footerCell->getRect();
		innerRect.bottom = rc.top - 1;
	}

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
		const Point clientPosition = event->getPosition() - m_scrollOffset;
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
		const Point clientPosition = event->getPosition() - m_scrollOffset;
		hitItem->mouseDoubleClick(event, clientPosition);
	}
}

void AutoWidget::eventMouseMove(MouseMoveEvent* event)
{
	Ref< AutoWidgetCell > hitItem = hitTest(event->getPosition());
	if (m_hoverCell != hitItem)
	{
		Rect updateRect;
		if (m_hoverCell != nullptr)
		{
			m_hoverCell->mouseLeave();
			updateRect = m_hoverCell->getRect();
		}
		if ((m_hoverCell = hitItem) != nullptr)
		{
			m_hoverCell->mouseEnter();
			updateRect = updateRect.contain(m_hoverCell->getRect());
		}
		update(&updateRect);
	}

	if (m_captureCell)
	{
		const Point clientPosition = event->getPosition() - m_scrollOffset;
		m_captureCell->mouseMove(event, clientPosition);
	}

	if (m_focusCell)
	{
		const Point clientPosition = event->getPosition() - m_scrollOffset;
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

void AutoWidget::eventMouseTrack(MouseTrackEvent* event)
{
	if (!event->entered() && m_hoverCell != nullptr)
	{
		m_hoverCell->mouseLeave();
		m_hoverCell = nullptr;
		update();
	}
}

void AutoWidget::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect innerRect = event->getUpdateRect();
	const StyleSheet* ss = getStyleSheet();

	const bool enabled = isEnable(true);
	canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
	canvas.setBackground(ss->getColor(this, enabled ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(innerRect);

	for (const auto& instance : m_cells)
	{
		const Rect rc = instance->getRect().offset(m_scrollOffset);
		if (rc.intersect(innerRect))
			instance->paint(canvas, rc);
	}

	if (m_headerCell)
	{
		const Rect rc = m_headerCell->getRect().offset(m_scrollOffset.cx, 0);
		if (rc.intersect(innerRect))
			m_headerCell->paint(canvas, rc);
	}

	if (m_footerCell)
	{
		const Rect rc = m_footerCell->getRect().offset(m_scrollOffset.cx, 0);
		if (rc.intersect(innerRect))
			m_footerCell->paint(canvas, rc);
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
	for (auto i = m_intervals.begin(); i != m_intervals.end(); )
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
