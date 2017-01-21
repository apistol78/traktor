#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AutoWidgetCell", AutoWidgetCell, ui::EventSubject)

AutoWidgetCell::AutoWidgetCell()
:	m_widget(0)
{
}

AutoWidgetCell::~AutoWidgetCell()
{
	m_widget = 0;
}

void AutoWidgetCell::placeCells(AutoWidget* widget, const Rect& rect)
{
	m_widget = widget;
}

AutoWidgetCell* AutoWidgetCell::hitTest(const Point& position)
{
	return this;
}

bool AutoWidgetCell::beginCapture()
{
	return true;
}

void AutoWidgetCell::endCapture()
{
}

void AutoWidgetCell::interval()
{
}

void AutoWidgetCell::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseMove(MouseMoveEvent* event, const Point& position)
{
}

void AutoWidgetCell::mouseMoveFocus(MouseMoveEvent* event, const Point& position)
{
}

void AutoWidgetCell::paint(Canvas& canvas, const Rect& rect)
{
}

void AutoWidgetCell::requestUpdate()
{
	if (m_widget)
		m_widget->requestUpdate();
}

void AutoWidgetCell::raiseEvent(Event* event)
{
	if (m_widget)
		m_widget->raiseEvent(event);
}

		}
	}
}
