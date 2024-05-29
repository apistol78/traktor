/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/CheckBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CheckBox", CheckBox, Widget)

CheckBox::CheckBox()
{
	m_imageUnchecked = new StyleBitmap(L"UI.Unchecked");
	m_imageChecked = new StyleBitmap(L"UI.Checked");
}

bool CheckBox::create(Widget* parent, const std::wstring& text, bool checked)
{
	if (!Widget::create(parent, WsFocus))
		return false;

	setText(text);
	setChecked(checked);

	addEventHandler< PaintEvent >(this, &CheckBox::eventPaint);
	addEventHandler< MouseButtonDownEvent >(this, &CheckBox::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &CheckBox::eventButtonUp);

	return true;
}

void CheckBox::setChecked(bool checked)
{
	m_checked = checked;
}

bool CheckBox::isChecked() const
{
	return m_checked;
}

Size CheckBox::getPreferredSize(const Size& hint) const
{
	const int32_t height = getFontMetric().getHeight() + pixel(4_ut) * 2;
	const int32_t width = getFontMetric().getExtent(getText()).cx;
	return Size(
		width + m_imageUnchecked->getSize(this).cx + pixel(4_ut),
		std::max(height, m_imageUnchecked->getSize(this).cy)
	);
}

void CheckBox::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	IBitmap* image = m_checked ? m_imageChecked : m_imageUnchecked;
	T_ASSERT(image);

	const int32_t y = (rcInner.getHeight() - image->getSize(this).cy) / 2;

	canvas.drawBitmap(
		Point(0, y),
		Point(0, 0),
		image->getSize(this),
		image,
		BlendMode::Alpha
	);

	Rect rcText = rcInner;
	rcText.left += image->getSize(this).cx + pixel(4_ut);

	canvas.setForeground(ss->getColor(this, isEnable(true) ? L"color" : L"color-disabled"));
	canvas.drawText(rcText, getText(), AnLeft, AnCenter);

	event->consume();
}

void CheckBox::eventButtonDown(MouseButtonDownEvent* event)
{
	setFocus();
	setCapture();
}

void CheckBox::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	releaseCapture();

	if (getInnerRect().inside(event->getPosition()))
	{
		m_checked = !m_checked;
		update();

		ButtonClickEvent clickEvent(this);
		raiseEvent(&clickEvent);
	}
}

}
