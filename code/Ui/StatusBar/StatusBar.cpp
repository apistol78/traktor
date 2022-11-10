/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Form.h"
#include "Ui/StyleSheet.h"
#include "Ui/StatusBar/StatusBar.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_preferedHeight = 23;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StatusBar", StatusBar, Widget)

bool StatusBar::create(Widget* parent, int style)
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
	Size preferedSize(0, dpi96(c_preferedHeight));
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

		rc.top += dpi96(3);
		rc.bottom -= dpi96(3);
		rc.right -= dpi96(20);
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
				w = canvas.getFontMetric().getExtent(m_columns[i].text).cx + ui::dpi96(8);

			Rect rcText = rc;
			rcText.left = x;
			rcText.right = x + w;
			canvas.drawText(rcText.inflate(dpi96(-8), 0), m_columns[i].text, AnLeft, AnCenter);

			x += w;
		}
	}

	// Draw scale grip if parent is a form and isn't maximized.
	Form* parentForm = dynamic_type_cast< Form* >(getParent());
	if (parentForm && !parentForm->isMaximized())
	{
		rc = rc.inflate(-dpi96(2), -dpi96(2));
		for (int i = 0; i <= 2; ++i)
		{
			canvas.setForeground(Color4ub(180, 180, 180));
			canvas.drawLine(rc.right, rc.bottom - dpi96(i * 4 - 1), rc.right - dpi96(i * 4 - 1), rc.bottom);
			canvas.drawLine(rc.right, rc.bottom - dpi96(i * 4 - 2), rc.right - dpi96(i * 4 - 2), rc.bottom);

			canvas.setForeground(Color4ub(255, 255, 255));
			canvas.drawLine(rc.right, rc.bottom - dpi96(i * 4 - 3), rc.right - dpi96(i * 4 - 3), rc.bottom);
		}
	}

	event->consume();
}

	}
}
