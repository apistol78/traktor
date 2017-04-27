/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Custom/ToolTip.h"
#include "Ui/Custom/ToolTipEvent.h"
#include "Ui/Itf/IEventLoop.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_margin = 4;
const int c_cursorHeight = 16;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolTip", ToolTip, ToolForm)

ToolTip::ToolTip()
:	m_tracking(false)
,	m_counter(0)
{
}

bool ToolTip::create(Widget* parent)
{
	if (!ToolForm::create(parent, L"", 0, 0, WsTop))
		return false;

	addEventHandler< TimerEvent >(this, &ToolTip::eventTimer);
	addEventHandler< PaintEvent >(this, &ToolTip::eventPaint);

	setFont(parent->getFont());
	startTimer(100);

	return true;
}

void ToolTip::show(const Point& at, const std::wstring& text)
{
	T_ASSERT (!text.empty());

	setText(text);

	Size extent = getTextExtent(text);
	extent.cx += ui::scaleBySystemDPI(c_margin) * 2;
	extent.cy += ui::scaleBySystemDPI(c_margin) * 2;

	Point tipPosition = getParent()->clientToScreen(at);
	tipPosition.y += ui::scaleBySystemDPI(c_cursorHeight);

	setRect(Rect(tipPosition, extent));

	setVisible(true);
}

void ToolTip::eventTimer(TimerEvent* event)
{
	Ref< Widget > parent = getParent();

	if (!parent->isVisible(true))
	{
		setVisible(false);
		return;
	}

	Point mousePosition = parent->getMousePosition();
	bool inside = false;

	if (mousePosition.x >= 0 && mousePosition.y >= 0)
	{
		Size parentSize = parent->getRect().getSize();
		if (mousePosition.x < parentSize.cx && mousePosition.y < parentSize.cy)
			inside = true;
	}

	inside &= parent->hitTest(parent->clientToScreen(mousePosition));

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

	innerRect = innerRect.inflate(-ui::scaleBySystemDPI(c_margin), 0);
	canvas.drawText(innerRect, getText(), AnLeft, AnCenter);

	event->consume();
}

		}
	}
}
