#import "Ui/Cocoa/NSOutlineViewDelegateProxy.h"

@implementation NSOutlineViewDelegateProxy

- (void) setCallback: (traktor::ui::INSOutlineViewEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void) outlineViewSelectionDidChange: (NSNotification*)notification
{
	m_eventsCallback->event_selectionDidChange();
}

@end
