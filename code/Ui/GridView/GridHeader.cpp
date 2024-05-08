/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/GridView/GridHeader.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridView.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridHeader", GridHeader, AutoWidgetCell)

void GridHeader::setColumns(const RefArray< GridColumn >& columns)
{
	m_columns = columns;
}

void GridHeader::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	if (event->getButton() != MbtLeft || m_columns.size() < 2)
		return;

	int32_t dx = pixel(2_ut);
	int32_t x = 0;

	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];
		x += pixel(column->getWidth());
		if (position.x >= x - dx && position.x <= x + dx)
		{
			m_resizeColumn = column;
			m_resizeWidth = pixel(column->getWidth());
			m_resizePosition = x;
			break;
		}
	}
}

void GridHeader::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	m_resizeColumn = 0;
}

void GridHeader::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
	if (event->getButton() != MbtLeft || m_columns.size() < 2)
		return;

	int32_t dx = pixel(2_ut);
	int32_t x = 0;

	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];
		x += pixel(column->getWidth());
		if (position.x >= x - dx && position.x <= x + dx)
		{
			getWidget< GridView >()->fitColumn(i);
			break;
		}
	}
}

void GridHeader::mouseMove(MouseMoveEvent* event, const Point& position)
{
	if (!m_resizeColumn)
		return;

	int32_t width = m_resizeWidth + position.x - m_resizePosition;
	if (width < 16)
		width = 16;

	m_resizeColumn->setWidth(getWidget()->unit(width));
	getWidget< AutoWidget >()->requestUpdate();
}

void GridHeader::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(Rect(0, rect.top, rect.getWidth(), rect.bottom));

	int32_t left = rect.left;
	for (uint32_t i = 0; i < m_columns.size(); ++i)
	{
		GridColumn* column = m_columns[i];

		int32_t width = pixel(column->getWidth());
		if (m_columns.size() == 1)
			width = rect.getWidth();

		Rect rcText(left + 2, rect.top, left + width - 2, rect.bottom);

		canvas.setClipRect(rcText);

		canvas.setForeground(ss->getColor(this, getWidget< AutoWidget >()->isEnable(true) ? L"color" : L"color-disabled"));
		canvas.drawText(rcText, column->getTitle(), AnLeft, AnCenter);

		canvas.resetClipRect();

		canvas.setForeground(ss->getColor(this, L"line-color"));
		canvas.drawLine(left, rect.top + 4, left, rect.bottom - 4);

		left += width;
	}

	canvas.setForeground(ss->getColor(this, L"line-color"));
	canvas.drawLine(left, rect.top + 4, left, rect.bottom - 4);
}

}
