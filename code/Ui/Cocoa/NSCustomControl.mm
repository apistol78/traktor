#include "Ui/Cocoa/NSCustomControl.h"
#include "Ui/Cocoa/NSCustomCell.h"
#include "Core/Log/Log.h"

@implementation NSCustomControl

- (id) initWithFrame: (NSRect)frameRect
{
	self = [super initWithFrame: frameRect];
	m_string = 0;
	return self;
}

- (void) setCallback: (traktor::ui::INSControlEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void) setStringValue: (NSString*)aString
{
	if (m_string)
		[m_string release];

	m_string = aString;
	
	if (m_string)
		[m_string retain];
}

- (NSString*) stringValue
{
	return m_string;
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

@end
