#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Custom/Auto/AutoWidget.h"
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

void GridHeaderCell::mouseDown(const Point& position)
{
	if (m_columns.size() < 2)
		return;

	int32_t x = 0;
	for (uint32_t i = 0; i < m_columns.size() - 1; ++i)
	{
		GridColumn* column = m_columns[i];
		x += column->getWidth();

		if (position.x >= x - 1 && position.x <= x + 1)
		{
			m_resizeColumn = column;
			m_resizeWidth = column->getWidth();
			m_resizePosition = x;
			break;
		}
	}
}

void GridHeaderCell::mouseUp(const Point& position)
{
	m_resizeColumn = 0;
}

void GridHeaderCell::mouseMove(const Point& position)
{
	if (!m_resizeColumn)
		return;

	int32_t width = m_resizeWidth + position.x - m_resizePosition;
	if (width < 16)
		width = 16;

	m_resizeColumn->setWidth(width);
	getWidget()->requestUpdate();
}

void GridHeaderCell::paint(Canvas& canvas, const Rect& rect)
{
	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillGradientRect(Rect(0, rect.top, rect.getWidth(), rect.bottom));

	int32_t left = rect.left;
	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];

		int32_t width = column->getWidth();
		if (m_columns.size() == 1)
			width = rect.getWidth();

		canvas.setForeground(getWidget()->isEnable() ? getSystemColor(ScWindowText) : getSystemColor(ScDisabledText));
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
