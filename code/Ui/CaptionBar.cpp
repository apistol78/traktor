/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/CaptionBar.h"
#include "Ui/Form.h"
#include "Ui/MiniButton.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_preferedHeightMargin = 8_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CaptionBar", CaptionBar, Widget)

bool CaptionBar::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_buttonMinimize = new MiniButton();
	m_buttonMinimize->create(this, new ui::StyleBitmap(L"UI.CaptionMinimize"), MiniButton::WsNoBorder);
	m_buttonMinimize->addEventHandler< ButtonClickEvent >(this, &CaptionBar::eventButtonClick);

	m_buttonMaximizeOrRestore = new MiniButton();
	m_buttonMaximizeOrRestore->create(this, new ui::StyleBitmap(L"UI.CaptionMaximize"), MiniButton::WsNoBorder);
	m_buttonMaximizeOrRestore->addEventHandler< ButtonClickEvent >(this, &CaptionBar::eventButtonClick);

	m_buttonClose = new MiniButton();
	m_buttonClose->create(this, new ui::StyleBitmap(L"UI.CaptionClose"), MiniButton::WsNoBorder);
	m_buttonClose->addEventHandler< ButtonClickEvent >(this, &CaptionBar::eventButtonClick);

	addEventHandler< MouseButtonDownEvent >(this, &CaptionBar::eventMouseButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &CaptionBar::eventMouseButtonUp);
	addEventHandler< MouseMoveEvent >(this, &CaptionBar::eventMouseMove);
	addEventHandler< SizeEvent >(this, &CaptionBar::eventSize);
	addEventHandler< PaintEvent >(this, &CaptionBar::eventPaint);

	return true;
}

Size CaptionBar::getPreferredSize(const Size& hint) const
{
	Size preferedSize(0, getFontMetric().getHeight() + pixel(c_preferedHeightMargin) * 2);
	if (getParent())
		preferedSize.cx = getParent()->getInnerRect().getWidth();
	return preferedSize;
}

void CaptionBar::eventButtonClick(ButtonClickEvent* event)
{
	Form* parentForm = dynamic_type_cast< Form* >(getParent());
	if (!parentForm)
		return;

	if (event->getSender() == m_buttonMinimize)
		parentForm->minimize();
	else if (event->getSender() == m_buttonMaximizeOrRestore)
	{
		if (!parentForm->isMaximized())
			parentForm->maximize();
		else
			parentForm->restore();
	}
	else if (event->getSender() == m_buttonClose)
	{
		CloseEvent closeEvent(parentForm);
		parentForm->raiseEvent(&closeEvent);
		if (!closeEvent.consumed() && !closeEvent.cancelled())
			parentForm->destroy();
	}
}

void CaptionBar::eventMouseButtonDown(MouseButtonDownEvent* event)
{
	m_mousePosition = getParent()->clientToScreen(event->getPosition());
	m_parentRect = getParent()->getRect();

	setCapture();
}

void CaptionBar::eventMouseButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
}

void CaptionBar::eventMouseMove(MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	const Point position = getParent()->clientToScreen(event->getPosition());
	const Rect rc = m_parentRect.offset(position - m_mousePosition);
	getParent()->setRect(rc);
}

void CaptionBar::eventSize(SizeEvent* event)
{
	const Rect rc = getInnerRect();
	const int32_t pad = pixel(4_ut);

	int32_t r = rc.right - pad;
	int32_t h = rc.getHeight();

	{
		const Size sz = m_buttonClose->getPreferredSize(Size(0, 0));
		m_buttonClose->setRect({ Point(r - sz.cx, (h - sz.cy) / 2), Size(sz.cx, sz.cy) });
		r -= sz.cx + pad;
	}

	{
		const Size sz = m_buttonMaximizeOrRestore->getPreferredSize(Size(0, 0));
		m_buttonMaximizeOrRestore->setRect({ Point(r - sz.cx, (h - sz.cy) / 2), Size(sz.cx, sz.cy) });
		r -= sz.cx + pad;
	}

	{
		const Size sz = m_buttonMinimize->getPreferredSize(Size(0, 0));
		m_buttonMinimize->setRect({ Point(r - sz.cx, (h - sz.cy) / 2), Size(sz.cx, sz.cy) });
		r -= sz.cx + pad;
	}
}

void CaptionBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	const std::wstring text = getParent()->getText();
	canvas.setForeground(ss->getColor(this, L"color"));

	Font font = getFont();
	font.setBold(true);
	font.setSize(font.getSize() + 2_ut);
	canvas.setFont(font);

	Rect rcText = rc;
	rcText.left = pixel(8_ut);
	rcText.right = rc.right;
	canvas.drawText(rcText, text, AnLeft, AnCenter);

	event->consume();
}

}
