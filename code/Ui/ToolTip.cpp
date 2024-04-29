/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/ToolTip.h"
#include "Ui/ToolTipEvent.h"
#include "Ui/Itf/IEventLoop.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_margin = 4_ut;
const Unit c_cursorHeight = 16_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolTip", ToolTip, ToolForm)

bool ToolTip::create(Widget* parent)
{
	if (!ToolForm::create(parent, L"", 0_ut, 0_ut, WsTop))
		return false;

	addEventHandler< TimerEvent >(this, &ToolTip::eventTimer);
	addEventHandler< PaintEvent >(this, &ToolTip::eventPaint);

	setFont(parent->getFont());
	startTimer(100);

	return true;
}

void ToolTip::show(const Point& at, const std::wstring& text)
{
	T_ASSERT(!text.empty());

	setText(text);

	Size extent = getFontMetric().getExtent(text);
	extent.cx += pixel(c_margin) * 2;
	extent.cy += pixel(c_margin) * 2;

	Point tipPosition = getParent()->clientToScreen(at);
	tipPosition.y += pixel(c_cursorHeight);

	setRect(Rect(tipPosition, extent));

	setVisible(true);
}

void ToolTip::eventTimer(TimerEvent* event)
{
	Ref< Widget > parent = getParent();

	if (!parent->isVisible(true) || !parent->isForeground())
	{
		setVisible(false);
		return;
	}

	Point mousePosition = parent->getMousePosition();
	bool inside = false;

	if (mousePosition.x >= 0 && mousePosition.y >= 0)
	{
		const Size parentSize = parent->getRect().getSize();
		if (mousePosition.x < parentSize.cx && mousePosition.y < parentSize.cy)
			inside = true;
	}

	inside &= parent->getRect().inside(mousePosition);

	if (inside && !isVisible(true))
	{
		if (!m_tracking)
		{
			// Start tracking mouse; don't show tooltip until some time has passed.
			m_tracking = true;
			m_counter = 0;
		}
		else if (++m_counter >= 4)
		{
			// Issue tooltip event; event target should show the actual tooltip.
			ToolTipEvent tipEvent(this, mousePosition);
			raiseEvent(&tipEvent);

			m_tracking = false;
		}
	}
	else if (!inside && isVisible(true))
	{
		// Hide tooltip if mouse is outside of parent widget.
		setVisible(false);
	}
}

void ToolTip::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	Rect innerRect = getInnerRect();

	canvas.setForeground(Color4ub(80, 80, 40));
	canvas.setBackground(Color4ub(255, 255, 180));

	canvas.fillRect(innerRect);
	canvas.drawRect(innerRect);

	innerRect = innerRect.inflate(-pixel(c_margin), 0);
	canvas.drawText(innerRect, getText(), AnLeft, AnCenter);

	event->consume();
}

}
