#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/EventSubject.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

EventLoopCocoa::EventLoopCocoa()
:	m_exitCode(0)
,	m_terminated(false)
{
	m_pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
}

EventLoopCocoa::~EventLoopCocoa()
{
	[m_pool release];
}

bool EventLoopCocoa::process(EventSubject* owner)
{
	return false;
}

int EventLoopCocoa::execute(EventSubject* owner)
{
	const double c_shortInterval = 1.0 / 60.0;
	const double c_longInterval = 1.0;

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	[NSApp finishLaunching];
	
	double interval = c_shortInterval;
	while (!m_terminated)
	{
		[pool release];
		pool = [[NSAutoreleasePool alloc] init];
	
		NSDate* untilDate = [[NSDate alloc] initWithTimeIntervalSinceNow: interval];
	
		// Get application events.
		NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: untilDate inMode: NSDefaultRunLoopMode dequeue: YES];
		if (event)
		{
			[NSApp sendEvent: event];
			[NSApp updateWindows];
			continue;
		}
		
		// No event queued; kick off idle.
		IdleEvent idleEvent(owner);
		owner->raiseEvent(EiIdle, &idleEvent);
		if (idleEvent.requestedMore())
			interval = c_shortInterval;
		else
			interval = c_longInterval;
	}
	
	[pool release];
	
	return m_exitCode;
}

void EventLoopCocoa::exit(int exitCode)
{
	m_exitCode = exitCode;
	m_terminated = true;
}

int EventLoopCocoa::getExitCode() const
{
	return m_exitCode;
}

int EventLoopCocoa::getAsyncKeyState() const
{
	return KsNone;
}

	}
}
