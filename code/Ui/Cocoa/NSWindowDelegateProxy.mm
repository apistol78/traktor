#import "Ui/Cocoa/NSWindowDelegateProxy.h"

@implementation NSWindowDelegateProxy

- (id) init
{
	if ((self = [super init]) != nil)
		m_eventsCallback = nullptr;
	return self;
}

- (id) initWithCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback
{
	if ((self = [super init]) != nil)
		m_eventsCallback = eventsCallback;
	return self;
}

- (void)setCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback
{
	m_eventsCallback = eventsCallback;
}

- (void)windowDidMove: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidMove();
}

- (void)windowDidResize: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidResize();
}

- (BOOL)windowShouldClose: (id)sender
{
	return m_eventsCallback->event_windowShouldClose();
}

- (void)windowDidBecomeKey: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidBecomeKey();
}

- (void)windowDidResignKey: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidResignKey();
}

- (void)windowDidBecomeMain: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidBecomeMain();
}

- (void)windowDidResignMain: (NSNotification*)notification
{
	m_eventsCallback->event_windowDidResignMain();
}

@end
