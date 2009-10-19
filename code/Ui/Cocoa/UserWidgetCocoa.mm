#include "Ui/Cocoa/UserWidgetCocoa.h"
#include "Ui/Cocoa/CanvasCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/EventSubject.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace ui
	{
	
UserWidgetCocoa::UserWidgetCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IUserWidget, NSCustomControl >(owner)
{
}

void UserWidgetCocoa::destroy()
{
	if (m_control)
		[m_control setCallback: nil];

	WidgetCocoaImpl< IUserWidget, NSCustomControl >::destroy();
}

bool UserWidgetCocoa::create(IWidget* parent, int style)
{
	m_control = [[NSCustomControl alloc]
		initWithFrame: NSMakeRect(0, 0, 0, 0)
	];
	[m_control setCallback: this];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

	return true;
}

bool UserWidgetCocoa::event_drawRect(const NSRect& rect)
{
	if (!m_owner->hasEventHandler(EiPaint))
		return false;

	Rect rc = fromNSRect(rect);

	CanvasCocoa canvasImpl(m_control);
	Canvas canvas(&canvasImpl);
	PaintEvent paintEvent(m_owner, (Object*)0, canvas, rc);
	m_owner->raiseEvent(EiPaint, &paintEvent);

	return paintEvent.consumed();
}

bool UserWidgetCocoa::event_viewDidEndLiveResize()
{
	raiseSizeEvent();
	return true;
}

bool UserWidgetCocoa::event_mouseDown(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler(EiButtonDown))
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	if (button == 1)
		button = MouseEvent::BtLeft;
	else if (button == 2)
		button = MouseEvent::BtRight;

	MouseEvent mouseEvent(
		m_owner,
		0,
		button,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(EiButtonDown, &mouseEvent);
	
	return true;
}

bool UserWidgetCocoa::event_mouseUp(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler(EiButtonUp))
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];
	
	if (button == 1)
		button = MouseEvent::BtLeft;
	else if (button == 2)
		button = MouseEvent::BtRight;
	
	MouseEvent mouseEvent(
		m_owner,
		0,
		button,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(EiButtonUp, &mouseEvent);
	
	return true;
}
	
bool UserWidgetCocoa::event_mouseMoved(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler(EiMouseMove))
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	if (button == 1)
		button = MouseEvent::BtLeft;
	else if (button == 2)
		button = MouseEvent::BtRight;

	MouseEvent mouseEvent(
		m_owner,
		0,
		button,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(EiMouseMove, &mouseEvent);
	
	return true;
}

	}
}
