#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/NSDebugAutoreleasePool.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/EventSubject.h"
#include "Ui/Enums.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

EventLoopCocoa::EventLoopCocoa()
:	m_launching(true)
,	m_exitCode(0)
,	m_terminated(false)
,	m_modifierFlags(0)
,	m_idleMode(true)
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
	if (m_launching)
	{
		[NSApp finishLaunching];
		m_launching = false;
	}

	if (m_terminated)
		return false;

	// Process a single event.
	{
		NSDebugAutoreleasePool* pool = [[NSDebugAutoreleasePool alloc] init];

		if (m_idleMode)
		{
			// Poll application events.
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event != nil)
			{
				if (!handleGlobalEvents(owner, event))
				{
					// Process event.
					[NSApp sendEvent: event];
					[NSApp updateWindows];
				}
			}
			else
			{
				// No event queued; kick off idle.
				IdleEvent idleEvent(owner);
				owner->raiseEvent(EiIdle, &idleEvent);
				if (!idleEvent.requestedMore())
					m_idleMode = false;
			}
		}
		else
		{
			// Get application events.
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: [NSDate distantFuture] inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event != nil)
			{
				if (!handleGlobalEvents(owner, event))
				{
					// Process event.
					[NSApp sendEvent: event];
					[NSApp updateWindows];
				}
			}
			m_idleMode = true;
		}
		
		[pool release];
	}
	
	return true;
}

int EventLoopCocoa::execute(EventSubject* owner)
{
	if (m_launching)
	{
		[NSApp finishLaunching];
		m_launching = false;
	}
		
	while (!m_terminated)
	{
		NSDebugAutoreleasePool* pool = [[NSDebugAutoreleasePool alloc] init];

		if (m_idleMode)
		{
			// Poll application events.
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event != nil)
			{
				if (!handleGlobalEvents(owner, event))
				{
					// Process event.
					[NSApp sendEvent: event];
					[NSApp updateWindows];
				}
			}
			else
			{
				// No event queued; kick off idle.
				IdleEvent idleEvent(owner);
				owner->raiseEvent(EiIdle, &idleEvent);
				if (!idleEvent.requestedMore())
					m_idleMode = false;
			}
		}
		else
		{
			// Get application events.
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: [NSDate distantFuture] inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event != nil)
			{
				if (!handleGlobalEvents(owner, event))
				{
					// Process event.
					[NSApp sendEvent: event];
					[NSApp updateWindows];
				}
			}
			m_idleMode = true;
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
	if (m_modifierFlags & NSCommandKeyMask)
		keyState |= KsCommand;

	return keyState;
}

bool EventLoopCocoa::handleGlobalEvents(EventSubject* owner, NSEvent* event)
{
	// Record modifier flags.
	m_modifierFlags = [event modifierFlags];
	
	// Process key events; if they are globally consumed we don't dispatch further.
	NSEventType eventType = [event type];
	if (eventType == NSKeyDown || eventType == NSKeyUp)
	{
		uint32_t systemKey = [event keyCode];
		VirtualKey virtualKey = translateKeyCode(systemKey);
		std::wstring chs = fromNSString([event characters]);
		
		KeyEvent keyEvent(
			owner,
			0,
			virtualKey,
			systemKey,
			chs.empty() ? 0 : chs[0]
		);
		owner->raiseEvent(
			eventType == NSKeyDown ? EiKeyDown : EiKeyUp,
			&keyEvent
		);
		
		return keyEvent.consumed();
	}

	return false;
}

	}
}
