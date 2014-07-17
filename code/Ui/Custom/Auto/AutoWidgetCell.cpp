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

void AutoWidgetCell::mouseDown(const Point& position)
{
}

void AutoWidgetCell::mouseUp(const Point& position)
{
}

void AutoWidgetCell::mouseMove(const Point& position)
{
}

void AutoWidgetCell::paint(Canvas& canvas, const Rect& rect)
{
}

AutoWidget* AutoWidgetCell::getWidget()
{
	return m_widget;
}

		}
	}
}
