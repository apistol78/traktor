#include "Ui/Custom/ToolTip.h"
#include "Ui/Custom/ToolTipEvent.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Itf/IEventLoop.h"
#include "Core/Log/Log.h"

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

	addTimerEventHandler(createMethodHandler(this, &ToolTip::eventTimer));
	addPaintEventHandler(createMethodHandler(this, &ToolTip::eventPaint));

	setFont(parent->getFont());
	startTimer(100);

	return true;
}

void ToolTip::show(const Point& at, const std::wstring& text)
{
	T_ASSERT (!text.empty());

	setText(text);

	Size extent = getTextExtent(text);
	extent.cx += c_margin * 2;
	extent.cy += c_margin * 2;

	Point tipPosition = getParent()->clientToScreen(at);
	tipPosition.y += c_cursorHeight;

	setRect(Rect(tipPosition, extent));

	setVisible(true);
}

void ToolTip::addShowEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiShowTip, eventHandler);
}

void ToolTip::eventTimer(Event* event)
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
			raiseEvent(EiShowTip, &tipEvent);

			m_tracking = false;
		}
	}
	else if (!inside && isVisible(true))
	{
		// Hide tooltip if mouse is outside of parent widget.
		setVisible(false);
	}
}

void ToolTip::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Rect innerRect = getInnerRect();
	
	canvas.setForeground(Color(80, 80, 40));
	canvas.setBackground(Color(255, 255, 180));

	canvas.fillRect(innerRect);
	canvas.drawRect(innerRect);

	innerRect = innerRect.inflate(-c_margin, 0);
	canvas.drawText(innerRect, getText(), AnLeft, AnCenter);

	paintEvent->consume();
}

		}
	}
}
