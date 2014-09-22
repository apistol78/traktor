#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/UserWidgetCocoa.h"
#include "Ui/Cocoa/CanvasCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/AllEvents.h"

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
	if (!m_owner->hasEventHandler< PaintEvent >())
		return false;

	Rect rc = fromNSRect(rect);
	
	NSFont* font = [m_control font];
	if (!font)
		return false;

	CanvasCocoa canvasImpl(m_control, font);
	Canvas canvas(&canvasImpl);
	PaintEvent paintEvent(m_owner, canvas, rc);
	m_owner->raiseEvent(&paintEvent);

	return paintEvent.consumed();
}

bool UserWidgetCocoa::event_viewDidEndLiveResize()
{
	raiseSizeEvent();
	return true;
}

bool UserWidgetCocoa::event_mouseDown(NSEvent* theEvent, int button)
{
	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	if (button == 1)
		button = MbtLeft;
	else if (button == 2)
		button = MbtRight;

	if ([theEvent clickCount] <= 1)
	{
		if (!m_owner->hasEventHandler< MouseButtonDownEvent >())
			return false;

		MouseButtonDownEvent mouseEvent(
			m_owner,
			button,
			fromNSPoint(mousePosition)
		);
		m_owner->raiseEvent(&mouseEvent);
	}
	else
	{
		if (!m_owner->hasEventHandler< MouseDoubleClickEvent >())
			return false;

		MouseDoubleClickEvent mouseEvent(
			m_owner,
			button,
			fromNSPoint(mousePosition)
		);
		m_owner->raiseEvent(&mouseEvent);
	}
	
	return true;
}

bool UserWidgetCocoa::event_mouseUp(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler< MouseButtonUpEvent >())
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];
	
	if (button == 1)
		button = MbtLeft;
	else if (button == 2)
		button = MbtRight;
	
	MouseButtonUpEvent mouseEvent(
		m_owner,
		button,
		fromNSPoint(mousePosition)
	);
	
	if ([theEvent clickCount] <= 1)
		m_owner->raiseEvent(&mouseEvent);
	
	return true;
}
	
bool UserWidgetCocoa::event_mouseMoved(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler< MouseMoveEvent >())
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	if (button == 1)
		button = MbtLeft;
	else if (button == 2)
		button = MbtRight;

	MouseMoveEvent mouseEvent(
		m_owner,
		button,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(&mouseEvent);
	
	return true;
}

bool UserWidgetCocoa::event_keyDown(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler< KeyDownEvent >())
		return false;
	
	NSString* chs = [theEvent characters];
	
	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;
	
	KeyDownEvent keyEvent(
		m_owner,
		translateKeyCode(keyCode),
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(&keyEvent);

	return true;
}

bool UserWidgetCocoa::event_keyUp(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler< KeyUpEvent >())
		return false;

	NSString* chs = [theEvent characters];
	
	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;
	
	KeyUpEvent keyEvent(
		m_owner,
		translateKeyCode(keyCode),
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(&keyEvent);

	return true;
}

bool UserWidgetCocoa::event_performKeyEquivalent(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler< KeyEvent >())
		return false;

	NSString* chs = [theEvent characters];
	if (!chs || [chs length] <= 0)
		return false;
	
	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = (wchar_t)[chs characterAtIndex: 0];
	
	KeyEvent keyEvent(
		m_owner,
		translateKeyCode(keyCode),
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(&keyEvent);

	return true;
}

	}
}
