#import "Ui/Cocoa/NSControlDelegateProxy.h"

@implementation NSControlDelegateProxy

- (void)setCallback: (traktor::ui::INSControlEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void)drawRect: (NSRect)rect
{
	m_eventsCallback->event_drawRect(rect);
}

- (void)viewDidEndLiveResize
{
	m_eventsCallback->event_viewDidEndLiveResize();
}

- (void) mouseDown: (NSEvent*)theEvent
{
	m_eventsCallback->event_mouseDown(theEvent);
}

- (void) mouseUp: (NSEvent*)theEvent;
{
	m_eventsCallback->event_mouseUp(theEvent);
}

- (void) mouseMoved: (NSEvent*)theEvent;
{
	m_eventsCallback->event_mouseMoved(theEvent);
}

@end
