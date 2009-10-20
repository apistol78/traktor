#import "Ui/Cocoa/NSOutlineViewDelegateProxy.h"

@implementation NSOutlineViewDelegateProxy

- (id) init
{
	if ((self = [super init]) != nil)
		m_eventsCallback = 0;
	return self;
}

- (void) setCallback: (traktor::ui::INSOutlineViewEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void) outlineViewSelectionDidChange: (NSNotification*)notification
{
	m_eventsCallback->event_selectionDidChange();
}

- (void) outlineViewRightMouseDown: (NSEvent*)event
{
	m_eventsCallback->event_rightMouseDown(event);
}

@end
