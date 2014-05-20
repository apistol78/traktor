#include <limits>
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/ScrollBar.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AutoWidget", AutoWidget, Widget)

AutoWidget::AutoWidget()
:	m_scrollOffset(0, 0)
,	m_deferredUpdate(false)
{
}

bool AutoWidget::create(ui::Widget* parent, int32_t style)
{
	if (!ui::Widget::create(parent, style))
		return false;

	addButtonDownEventHandler(createMethodHandler(this, &AutoWidget::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &AutoWidget::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &AutoWidget::eventMouseMove));
	addMouseWheelEventHandler(createMethodHandler(this, &AutoWidget::eventMouseWheel));
	addPaintEventHandler(createMethodHandler(this, &AutoWidget::eventPaint));
	addSizeEventHandler(createMethodHandler(this, &AutoWidget::eventSize));
	addTimerEventHandler(createMethodHandler(this, &AutoWidget::eventTimer));

	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarV = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBarH->addScrollEventHandler(createMethodHandler(this, &AutoWidget::eventScroll));
	m_scrollBarV->addScrollEventHandler(createMethodHandler(this, &AutoWidget::eventScroll));

	m_backgroundColor = getSystemColor(ScButtonFace);

	startTimer(100);
	return true;
}

void AutoWidget::setBackgroundColor(const Color4ub& color)
{
	m_backgroundColor = color;
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
	AutoWidgetCell* hit = 0;

	Point clientPosition = position - m_scrollOffset;
	for (std::vector< CellInstance >::reverse_iterator i = m_cells.rbegin(); i != m_cells.rend(); ++i)
	{
		if (!i->rc.inside(clientPosition))
			continue;

		if ((hit = i->cell->hitTest(this, clientPosition)) != 0)
			break;
	}

	return hit;
}

void AutoWidget::requestUpdate()
{
	m_deferredUpdate = true;
}

void AutoWidget::placeCell(AutoWidgetCell* cell, const Rect& rect)
{
	T_ASSERT (cell);
	
	// Add this cell instance.
	CellInstance instance = { cell, rect };
	m_cells.push_back(instance);

	// Notify cell to place sub-cells.
	cell->placeCells(this, rect);
}

void AutoWidget::updateLayout()
{
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

	for (std::vector< CellInstance >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		m_bounds.left = std::min(m_bounds.left, i->rc.left);
		m_bounds.right = std::max(m_bounds.right, i->rc.right);
		m_bounds.top = std::min(m_bounds.top, i->rc.top);
		m_bounds.bottom = std::max(m_bounds.bottom, i->rc.bottom);
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

	if (
		m_scrollBarH->isVisible(false) != scrollBarVisibleH ||
		m_scrollBarV->isVisible(false) != scrollBarVisibleV
	)
	{
		Rect innerRect = getInnerRect();

		if (m_scrollBarH->isVisible(false))
			innerRect.bottom -= m_scrollBarH->getPreferedSize().cy;
		if (m_scrollBarV->isVisible(false))
			innerRect.right -= m_scrollBarV->getPreferedSize().cx;

		m_cells.resize(0);
		layoutCells(innerRect);
	}

	if (!m_scrollBarH->isVisible(false))
		m_scrollOffset.cx = 0;
	if (!m_scrollBarV->isVisible(false))
		m_scrollOffset.cy = 0;
}

void AutoWidget::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	if (mouseEvent->getButton() != MouseEvent::BtLeft)
		return;

	Ref< AutoWidgetCell > hitItem = hitTest(mouseEvent->getPosition());
	if (hitItem)
	{
		m_focusCell = hitItem;
		if (hitItem->beginCapture(this))
		{
			Point clientPosition = mouseEvent->getPosition() - m_scrollOffset;
			m_captureCell = hitItem;
			m_captureCell->mouseDown(this, clientPosition);
			setCapture();
		}
	}
	else
		m_focusCell = 0;

	update();
}

void AutoWidget::eventButtonUp(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	if (mouseEvent->getButton() != MouseEvent::BtLeft)
		return;

	if (m_captureCell)
	{
		Point clientPosition = mouseEvent->getPosition() - m_scrollOffset;
		m_captureCell->mouseUp(this, clientPosition);
		m_captureCell->endCapture(this);
		releaseCapture();
		update();
	}
}

void AutoWidget::eventMouseMove(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	if (m_captureCell)
	{
		Point clientPosition = mouseEvent->getPosition() - m_scrollOffset;
		m_captureCell->mouseMove(this, clientPosition);
		update();
	}
}

void AutoWidget::eventMouseWheel(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);

	int32_t position = m_scrollBarV->getPosition();
	position -= mouseEvent->getWheelRotation() * 4;
	m_scrollBarV->setPosition(position);

	m_scrollOffset.cy = -m_scrollBarV->getPosition() * 16;
	update();
}

void AutoWidget::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent*, false >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Rect innerRect = getInnerRect();
	canvas.setBackground(m_backgroundColor);
	canvas.fillRect(innerRect);

	for (std::vector< CellInstance >::iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		Rect rc = i->rc.offset(m_scrollOffset);
		if (rc.intersect(innerRect))
			i->cell->paint(this, canvas, rc);
	}

	event->consume();
}

void AutoWidget::eventSize(Event* event)
{
	updateLayout();

	int32_t width = m_scrollBarV->getPreferedSize().cx;
	int32_t height = m_scrollBarH->getPreferedSize().cy;

	Rect innerRect = getInnerRect();

	int32_t widthH = innerRect.getWidth();
	if (m_scrollBarV->isVisible(false))
		widthH -= width;

	int32_t heightV = innerRect.getHeight();
	if (m_scrollBarH->isVisible(false))
		heightV -= height;

	m_scrollBarH->setRect(Rect(
		Point(0, innerRect.getHeight() - height),
		Size(widthH, height)
	));
	m_scrollBarV->setRect(Rect(
		Point(innerRect.getWidth() - width, 0),
		Size(width, heightV)
	));

	update();
}

void AutoWidget::eventTimer(Event* event)
{
	if (!isVisible(false))
		return;

	if (m_deferredUpdate)
	{
		m_deferredUpdate = false;
		updateLayout();
		update();
	}
}

void AutoWidget::eventScroll(Event* event)
{
	m_scrollOffset.cx = -m_scrollBarH->getPosition() * 16;
	m_scrollOffset.cy = -m_scrollBarV->getPosition() * 16;
	update();
}

		}
	}
}
