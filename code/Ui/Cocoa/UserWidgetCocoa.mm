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

bool UserWidgetCocoa::create(IWidget* parent, int style)
{
	m_control = [[NSCustomControl alloc]
		initWithFrame: NSMakeRect(0, 0, 0, 0)
	];
	
	if (style & WsClientBorder)
	{
		// @fixme
	}
	
	NSControlDelegateProxy* proxy = [[NSControlDelegateProxy alloc] init];
	[proxy setCallback: this];
	
	[m_control setDelegate: proxy];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

	return true;
}

bool UserWidgetCocoa::event_drawRect(const NSRect& rect)
{
	Rect rc = fromNSRect(m_control, rect);

	CanvasCocoa canvasImpl(m_control);
	Canvas canvas(&canvasImpl);
	PaintEvent paintEvent(m_owner, (Object*)0, canvas, rc);
	m_owner->raiseEvent(EiPaint, &paintEvent);

	return true;
}

bool UserWidgetCocoa::event_viewDidEndLiveResize()
{
	raiseSizeEvent();
	return true;
}

bool UserWidgetCocoa::event_mouseDown(NSEvent* theEvent)
{
	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	int button = 0;
	if (([theEvent buttonMask] & NSLeftMouseDownMask) != 0)
		button |= MouseEvent::BtLeft;
	if (([theEvent buttonMask] & NSRightMouseDownMask) != 0)
		button |= MouseEvent::BtRight;
	
	MouseEvent mouseEvent(
		m_owner,
		0,
		button,
		fromNSPoint(m_control, mousePosition)
	);
	m_owner->raiseEvent(EiButtonDown, &mouseEvent);
	
	return true;
}

bool UserWidgetCocoa::event_mouseUp(NSEvent* theEvent)
{
	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];
	
	int button = 0;
	if (([theEvent buttonMask] & NSLeftMouseDownMask) != 0)
		button |= MouseEvent::BtLeft;
	if (([theEvent buttonMask] & NSRightMouseDownMask) != 0)
		button |= MouseEvent::BtRight;

	MouseEvent mouseEvent(
		m_owner,
		0,
		button,
		fromNSPoint(m_control, mousePosition)
	);
	m_owner->raiseEvent(EiButtonUp, &mouseEvent);
	
	return true;
}
	
bool UserWidgetCocoa::event_mouseMoved(NSEvent* theEvent)
{
	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];
	
	int button = 0;
	if (([theEvent buttonMask] & NSLeftMouseDownMask) != 0)
		button |= MouseEvent::BtLeft;
	if (([theEvent buttonMask] & NSRightMouseDownMask) != 0)
		button |= MouseEvent::BtRight;
	
	MouseEvent mouseEvent(
		m_owner,
		0,
		button,
		fromNSPoint(m_control, mousePosition)
	);
	m_owner->raiseEvent(EiMouseMove, &mouseEvent);
	
	return true;
}

	}
}
