#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/NSDebugAutoreleasePool.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/EventSubject.h"
#include "Ui/Enums.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

EventLoopCocoa::EventLoopCocoa()
:	m_exitCode(0)
,	m_terminated(false)
,	m_modifierFlags(0)
{
	m_pool = [[NSDebugAutoreleasePool alloc] init];
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
	[NSApp finishLaunching];
		
	while (!m_terminated)
	{
		NSDebugAutoreleasePool* pool = [[NSDebugAutoreleasePool alloc] init];

		// Get application events.
		NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
		if (event != nil)
		{
			// Record modifier flags.
			m_modifierFlags = [event modifierFlags];
		
			// Process event.
			[NSApp sendEvent: event];
			[NSApp updateWindows];
		}
		else
		{
			// No event queued; kick off idle.
			IdleEvent idleEvent(owner);
			owner->raiseEvent(EiIdle, &idleEvent);
		}

		[pool release];
	}

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
	int keyState = KsNone;
	
	if (m_modifierFlags & NSControlKeyMask)
		keyState |= KsControl;
	if (m_modifierFlags & NSAlternateKeyMask)
		keyState |= KsMenu;
	if (m_modifierFlags & NSShiftKeyMask)
		keyState |= KsShift;

	return keyState;
}

	}
}
