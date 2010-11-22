#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Custom/GridView/GridHeaderCell.h"
#include "Ui/Custom/GridView/GridColumn.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridHeaderCell", GridHeaderCell, AutoWidgetCell)

void GridHeaderCell::setColumns(const RefArray< GridColumn >& columns)
{
	m_columns = columns;
}

void GridHeaderCell::mouseDown(AutoWidget* widget, const Point& position)
{
}

void GridHeaderCell::mouseUp(AutoWidget* widget, const Point& position)
{
}

void GridHeaderCell::mouseMove(AutoWidget* widget, const Point& position)
{
}

void GridHeaderCell::paint(AutoWidget* widget, Canvas& canvas, const Rect& rect)
{
	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillGradientRect(Rect(rect.left, rect.top, rect.right, rect.bottom));

	int32_t left = 0;
	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];

		int32_t width = column->getWidth();
		if (m_columns.size() == 1)
			width = rect.getWidth();

		canvas.setForeground(getSystemColor(ScWindowText));
		canvas.drawText(Rect(left + 2, rect.top, left + width - 2, rect.bottom), column->getTitle(), AnLeft, AnCenter);

		if (i > 0)
		{
			canvas.setForeground(Color4ub(208, 208, 208));
			canvas.drawLine(left, rect.top + 4, left, rect.bottom - 4);
		}

		left += width;
	}
}

		}
	}
}
