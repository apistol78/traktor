/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/MiniButton.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MiniButton", MiniButton, Widget)

bool MiniButton::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent))
		return false;

	m_pushed = false;
	setText(text);

	addEventHandler< MouseButtonDownEvent >(this, &MiniButton::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MiniButton::eventButtonUp);
	addEventHandler< PaintEvent >(this, &MiniButton::eventPaint);

	return true;
}

bool MiniButton::create(Widget* parent, IBitmap* image)
{
	if (!Widget::create(parent))
		return false;

	m_pushed = false;
	m_image  = image;

	addEventHandler< MouseButtonDownEvent >(this, &MiniButton::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MiniButton::eventButtonUp);
	addEventHandler< PaintEvent >(this, &MiniButton::eventPaint);

	return true;
}

void MiniButton::setImage(IBitmap* image)
{
	m_image = image;
}

Size MiniButton::getPreferredSize(const Size& hint) const
{
	if (m_image)
		return m_image->getSize() + Size(ui::dpi96(6), ui::dpi96(6));
	else
		return Size(getFontMetric().getExtent(getText()).cx + ui::dpi96(10), ui::dpi96(16));
}

Size MiniButton::getMaximumSize() const
{
	return getPreferredSize(Size(0, 0));
}

void MiniButton::eventButtonDown(MouseButtonDownEvent* event)
{
	m_pushed = true;
	update();

	setCapture();
	event->consume();
}

void MiniButton::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();

	if (m_pushed)
	{
		m_pushed = false;
		update();

		ButtonClickEvent clickEvent(this);
		raiseEvent(&clickEvent);
	}

	event->consume();
}

void MiniButton::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	if (isEnable())
	{
		canvas.setBackground(ss->getColor(this, m_pushed ? L"background-color-pushed" : L"background-color"));
		canvas.fillRect(rcInner);

		canvas.setForeground(ss->getColor(this, L"border-color"));
		canvas.drawRect(rcInner);

		if (m_pushed)
			rcInner = rcInner.offset(1, 1);
	}
	else
	{
		canvas.setBackground(ss->getColor(this, L"background-color-disabled"));
		canvas.fillRect(rcInner);

		canvas.setForeground(ss->getColor(this, L"border-color-disabled"));
		canvas.drawRect(rcInner);
	}

	if (m_image)
	{
		const Size size = m_image->getSize();
		const Size margin = rcInner.getSize() - size;
		const Point at(rcInner.left + margin.cx / 2, rcInner.top + margin.cy / 2);

		canvas.drawBitmap(
			at,
			Point(0, 0),
			size,
			m_image,
			BlendMode::Alpha
		);
	}
	else
	{
		canvas.setForeground(ss->getColor(this, L"color"));
		canvas.drawText(rcInner, getText(), AnCenter, AnCenter);
	}

	event->consume();
}

	}
}
