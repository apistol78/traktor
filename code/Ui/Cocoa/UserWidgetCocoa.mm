#include "Ui/Canvas.h"
#include "Ui/Cocoa/UserWidgetCocoa.h"
#include "Ui/Cocoa/CanvasCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/EventSubject.h"

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
	
	NSFont* font = [NSFont controlContentFontOfSize: 11.0];
	if (!font)
		return false;
	
	[m_control setFont: font];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

	return class_t::create();
}

bool UserWidgetCocoa::event_drawRect(const NSRect& rect)
{
	if (!m_owner->hasEventHandler(EiPaint))
		return false;

	Rect rc = fromNSRect(rect);
	
	NSFont* font = [m_control font];
	if (!font)
		return false;

	CanvasCocoa canvasImpl(m_control, font);
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
	
	if ([theEvent clickCount] <= 1)
		m_owner->raiseEvent(EiButtonDown, &mouseEvent);
	else
		m_owner->raiseEvent(EiDoubleClick, &mouseEvent);
	
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
	
	if ([theEvent clickCount] <= 1)
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

bool UserWidgetCocoa::event_keyDown(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler(EiKeyDown))
		return false;
	
	NSString* chs = [theEvent characters];
	
	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;
	
	KeyEvent keyEvent(
		m_owner,
		0,
		VkNull,
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(EiKeyDown, &keyEvent);

	return true;
}

bool UserWidgetCocoa::event_keyUp(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler(EiKeyUp))
		return false;

	NSString* chs = [theEvent characters];
	
	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;
	
	KeyEvent keyEvent(
		m_owner,
		0,
		VkNull,
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(EiKeyUp, &keyEvent);

	return true;
}

bool UserWidgetCocoa::event_performKeyEquivalent(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler(EiKey))
		return false;

	NSString* chs = [theEvent characters];
	
	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;
	
	KeyEvent keyEvent(
		m_owner,
		0,
		VkNull,
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(EiKey, &keyEvent);

	return true;
}

	}
}
