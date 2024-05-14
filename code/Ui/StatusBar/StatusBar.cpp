/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Form.h"
#include "Ui/StyleSheet.h"
#include "Ui/StatusBar/StatusBar.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_preferedHeightMargin = 2_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StatusBar", StatusBar, Widget)

bool StatusBar::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< SizeEvent >(this, &StatusBar::eventSize);
	addEventHandler< PaintEvent >(this, &StatusBar::eventPaint);
	return true;
}

void StatusBar::setAlert(bool alert)
{
	if (alert != m_alert)
	{
		m_alert = alert;
		update();
	}
}

void StatusBar::addColumn(int32_t width)
{
	auto& c = m_columns.push_back();
	c.width = width;
}

void StatusBar::setText(int32_t column, const std::wstring& text)
{
	if (column < 0 || column >= (int32_t)m_columns.size())
		return;

	auto& c = m_columns[column];
	if (c.text != text)
	{
		c.text = text;
		update();
	}
}

Size StatusBar::getPreferredSize(const Size& hint) const
{
	Size preferedSize(0, getFontMetric().getHeight() + pixel(c_preferedHeightMargin) * 2);
	if (getParent())
		preferedSize.cx = getParent()->getInnerRect().getWidth();
	return preferedSize;
}

void StatusBar::eventSize(SizeEvent* event)
{
	Ref< Widget > child = getFirstChild();
	if (child)
	{
		Rect rc = getInnerRect();
		Size preferedSize = child->getPreferredSize(rc.getSize());

		rc.top += pixel(3_ut);
		rc.bottom -= pixel(3_ut);
		rc.right -= pixel(20_ut);
		rc.left = rc.right - preferedSize.cx;

		child->setRect(rc);
	}
}

void StatusBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, m_alert ? L"background-color-alert" : L"background-color"));
	canvas.fillRect(rc);

	if (!m_columns.empty())
	{
		canvas.setForeground(ss->getColor(this, L"color"));

		const int32_t nc = (int32_t)m_columns.size();
		int32_t x = 0;
		for (int32_t i = 0; i < nc; ++i)
		{
			int32_t w = m_columns[i].width;
			if (w < 0 || i >= nc - 1)
				w = rc.getWidth() - x;
			else if (w == 0)
				w = canvas.getFontMetric().getExtent(m_columns[i].text).cx + pixel(8_ut);

			Rect rcText = rc;
			rcText.left = x;
			rcText.right = x + w;
			canvas.drawText(rcText.inflate(pixel(-8_ut), 0), m_columns[i].text, AnLeft, AnCenter);

			x += w;
		}
	}

	// Draw scale grip if parent is a form and isn't maximized.
	Form* parentForm = dynamic_type_cast< Form* >(getParent());
	if (parentForm && !parentForm->isMaximized())
	{
		rc = rc.inflate(-pixel(2_ut), -pixel(2_ut));
		for (int i = 0; i <= 2; ++i)
		{
			const Unit ii(i);

			canvas.setForeground(Color4ub(180, 180, 180));
			canvas.drawLine(rc.right, rc.bottom - pixel(4_ut * ii - 1_ut), rc.right - pixel(4_ut * ii - 1_ut), rc.bottom);
			canvas.drawLine(rc.right, rc.bottom - pixel(4_ut * ii - 2_ut), rc.right - pixel(4_ut * ii - 2_ut), rc.bottom);

			canvas.setForeground(Color4ub(255, 255, 255));
			canvas.drawLine(rc.right, rc.bottom - pixel(4_ut * ii - 3_ut), rc.right - pixel(4_ut * ii - 3_ut), rc.bottom);
		}
	}

	event->consume();
}

}
