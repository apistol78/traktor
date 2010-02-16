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
,	m_clientSize(0, 0)
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
	addPaintEventHandler(createMethodHandler(this, &AutoWidget::eventPaint));
	addSizeEventHandler(createMethodHandler(this, &AutoWidget::eventSize));
	addTimerEventHandler(createMethodHandler(this, &AutoWidget::eventTimer));

	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addScrollEventHandler(createMethodHandler(this, &AutoWidget::eventScroll));

	startTimer(100);
	return true;
}

void AutoWidget::setClientSize(const Size& size)
{
	m_clientSize = size;
	updateScrollBar();
	updateLayout();
	update();
}

AutoWidgetCell* AutoWidget::hitTest(const Point& position)
{
	AutoWidgetCell* hit = 0;
	for (RefArray< AutoWidgetCell >::iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		if ((hit = (*i)->hitTest(this, position)) != 0)
			break;
	}
	return hit;
}

void AutoWidget::requestUpdate()
{
	m_deferredUpdate = true;
}

void AutoWidget::addCell(AutoWidgetCell* cell)
{
	m_cells.push_back(cell);
}

void AutoWidget::removeAllCells()
{
	m_cells.resize(0);
}

const RefArray< AutoWidgetCell >& AutoWidget::getCells() const
{
	return m_cells;
}

void AutoWidget::updateScrollBar()
{
	if (m_clientSize.cy >= 0)
	{
		Rect innerRect = getInnerRect();

		int32_t rowCount = (m_clientSize.cy + 15) / 16;
		int32_t pageCount = (innerRect.getHeight() + 15) / 16;

		m_scrollBar->setRange(rowCount);
		m_scrollBar->setPage(pageCount);
		m_scrollBar->setVisible(rowCount > pageCount);
		m_scrollBar->update();
	}
	else
		m_scrollBar->setVisible(false);
}

void AutoWidget::updateLayout()
{
	Rect layoutRect = getInnerRect();

	if (m_clientSize.cx > 0)
		layoutRect.right = m_clientSize.cx;
	if (m_clientSize.cy > 0)
		layoutRect.bottom = m_clientSize.cy;

	if (m_scrollBar->isVisible(false))
		layoutRect.right -= m_scrollBar->getPreferedSize().cx;

	layoutCells(layoutRect);
}

void AutoWidget::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	if (mouseEvent->getButton() != MouseEvent::BtLeft)
		return;

	Point position = mouseEvent->getPosition() - m_scrollOffset;

	Ref< AutoWidgetCell > hitItem = hitTest(position);
	if (hitItem && hitItem->beginCapture(this))
	{
		m_captureItem = hitItem;
		m_captureItem->mouseDown(this, position);
		setCapture();
		update();
	}
}

void AutoWidget::eventButtonUp(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	if (mouseEvent->getButton() != MouseEvent::BtLeft)
		return;

	if (m_captureItem)
	{
		Point position = mouseEvent->getPosition() - m_scrollOffset;
		m_captureItem->mouseUp(this, position);
		m_captureItem->endCapture(this);
		releaseCapture();
		update();
	}
}

void AutoWidget::eventMouseMove(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	Point position = mouseEvent->getPosition() - m_scrollOffset;

	if (m_captureItem)
	{
		m_captureItem->mouseMove(this, position);
		update();
	}
}

void AutoWidget::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent*, false >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Rect innerRect = getInnerRect();
	canvas.fillRect(innerRect);

	for (RefArray< AutoWidgetCell >::iterator i = m_cells.begin(); i != m_cells.end(); ++i)
		(*i)->paint(this, canvas, m_scrollOffset);

	canvas.resetClipRect();
}

void AutoWidget::eventSize(Event* event)
{
	int32_t width = m_scrollBar->getPreferedSize().cx;

	Rect innerRect = getInnerRect();
	Rect scrollBarRect(Point(innerRect.getWidth() - width, 0), Size(width, innerRect.getHeight()));
	m_scrollBar->setRect(scrollBarRect);

	updateScrollBar();
	updateLayout();
}

void AutoWidget::eventTimer(Event* event)
{
	if (m_deferredUpdate)
	{
		m_deferredUpdate = false;
		update();
	}
}

void AutoWidget::eventScroll(Event* event)
{
	m_scrollOffset.cy = -m_scrollBar->getPosition() * 16;
	update();
}

		}
	}
}
