#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AutoWidgetCell", AutoWidgetCell, Object)

void AutoWidgetCell::placeCells(AutoWidget* widget, const Rect& rect)
{
}

AutoWidgetCell* AutoWidgetCell::hitTest(AutoWidget* widget, const Point& position)
{
	return this;
}

bool AutoWidgetCell::beginCapture(AutoWidget* widget)
{
	return true;
}

void AutoWidgetCell::endCapture(AutoWidget* widget)
{
}

void AutoWidgetCell::mouseDown(AutoWidget* widget, const Point& position)
{
}

void AutoWidgetCell::mouseUp(AutoWidget* widget, const Point& position)
{
}

void AutoWidgetCell::mouseMove(AutoWidget* widget, const Point& position)
{
}

void AutoWidgetCell::paint(AutoWidget* widget, Canvas& canvas, const Rect& rect)
{
}

		}
	}
}
