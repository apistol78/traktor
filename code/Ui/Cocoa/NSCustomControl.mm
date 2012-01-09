#include "Ui/Cocoa/NSCustomControl.h"
#include "Ui/Cocoa/NSCustomCell.h"
#include "Core/Log/Log.h"

@implementation NSCustomControl

- (id) initWithFrame: (NSRect)frameRect
{
	if ((self = [super initWithFrame: frameRect]) != nil)
	{
		m_string = nil;
		m_font = nil;
		m_eventsCallback = 0;
	}
	return self;
}

- (void) setCallback: (traktor::ui::INSControlEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void) setStringValue: (NSString*)aString
{
	if (m_string)
		[m_string autorelease];

	m_string = aString;
	
	if (m_string)
		[m_string retain];
}

- (NSString*) stringValue
{
	return m_string;
}

- (void) setFont: (NSFont*)font
{
	if (m_font)
		[m_font autorelease];
	
	m_font = font;
	
	if (m_font)
		[m_font retain];
}

- (NSFont*) font
{
	return m_font;
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) drawRect: (NSRect)rect
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_drawRect(rect);

	if (!consumed)
		[super drawRect: rect];
}

- (void) viewDidEndLiveResize
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_viewDidEndLiveResize();
		
	if (!consumed)
		[super viewDidEndLiveResize];
}

- (void) mouseDown: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseDown(theEvent, 1);

	if (!consumed)
		[super mouseDown: theEvent];
}

- (void) mouseUp: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseUp(theEvent, 1);

	if (!consumed)
		[super mouseUp: theEvent];
}

- (void) rightMouseDown: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseDown(theEvent, 2);

	if (!consumed)
		[super mouseDown: theEvent];
}

- (void) rightMouseUp: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseUp(theEvent, 2);

	if (!consumed)
		[super mouseUp: theEvent];
}

- (void) mouseMoved: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseMoved(theEvent, 0);

	if (!consumed)
		[super mouseMoved: theEvent];
}

- (void) mouseDragged: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseMoved(theEvent, 1);

	if (!consumed)
		[super mouseMoved: theEvent];
}

- (void) rightMouseDragged: (NSEvent*)theEvent
{
	bool consumed = false;

	if (m_eventsCallback)
		consumed = m_eventsCallback->event_mouseMoved(theEvent, 2);

	if (!consumed)
		[super mouseMoved: theEvent];
}

- (void) keyDown: (NSEvent*)theEvent
{
	bool consumed = false;
	
	if (m_eventsCallback)
		consumed = m_eventsCallback->event_keyDown(theEvent);
	
	if (!consumed)
		[super keyDown: theEvent];
}

- (void) keyUp: (NSEvent*)theEvent;
{
	bool consumed = false;
	
	if (m_eventsCallback)
		consumed = m_eventsCallback->event_keyUp(theEvent);
	
	if (!consumed)
		[super keyUp: theEvent];
}

@end
