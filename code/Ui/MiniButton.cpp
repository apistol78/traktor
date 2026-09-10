/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleConstants.h"
#include "Ui/StyleSheet.h"
#include "Ui/MiniButton.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MiniButton", MiniButton, Widget)

bool MiniButton::create(Widget* parent, const std::wstring& text, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	m_border = ((style & WsNoBorder) == 0);
	m_background = ((style & WsNoBackground) == 0);
	m_pushed = false;
	m_hover = false;
	setText(text);

	addEventHandler< MouseTrackEvent >(this, &MiniButton::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &MiniButton::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MiniButton::eventButtonUp);
	addEventHandler< PaintEvent >(this, &MiniButton::eventPaint);

	return true;
}

bool MiniButton::create(Widget* parent, IBitmap* image, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_border = ((style & WsNoBorder) == 0);
	m_background = ((style & WsNoBackground) == 0);
	m_pushed = false;
	m_hover = false;
	m_image  = image;

	addEventHandler< MouseTrackEvent >(this, &MiniButton::eventMouseTrack);
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
		return m_image->getSize(this) + Size(pixel(6_ut), pixel(6_ut));
	else
		return Size(getFontMetric().getExtent(getText()).cx + pixel(10_ut), pixel(16_ut));
}

Size MiniButton::getMaximumSize() const
{
	return getPreferredSize(Size(0, 0));
}

void MiniButton::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
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
	const StyleSheet* ss = getStyleSheet();
	const bool enabled = isEnable(true);

	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const int32_t radius = pixel(c_controlRadius);

	//// Cover the whole client area first; the rounded button leaves the four
	//// corners of the rectangle uncovered.
	//canvas.setBackground(ss->getColor(getParent(), L"background-color"));
	//canvas.fillRect(rcInner);

	// State colours belong to the button, not to whichever widget happens to
	// contain it; reading them from the parent resolved to the fallback colour.
	const wchar_t* background = nullptr;
	if (!enabled)
		background = m_background ? L"background-color-disabled" : nullptr;
	else if (m_background && m_pushed)
		background = L"background-color-pushed";
	else if (m_hover)
		background = L"background-color-hover";
	else if (m_background)
		background = L"background-color";

	if (background != nullptr)
	{
		canvas.setBackground(ss->getColor(this, background));
		canvas.fillRoundRect(rcInner, radius);
	}

	if (m_border)
	{
		canvas.setForeground(ss->getColor(this, enabled ? L"border-color" : L"border-color-disabled"));
		canvas.drawRoundRect(rcInner, radius);
	}

	if (m_image)
	{
		const Size size = m_image->getSize(this);
		const Size margin = rcInner.getSize() - size;
		const Point at(rcInner.left + margin.cx / 2, rcInner.top + margin.cy / 2);

		canvas.drawBitmap(
			at,
			Point(0, 0),
			size,
			m_image,
			BlendMode::Alpha,
			Filter::Linear,
			enabled ? 255 : 80
		);
	}
	else
	{
		canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
		canvas.drawText(rcInner, getText(), AnCenter, AnCenter);
	}

	event->consume();
}

}
