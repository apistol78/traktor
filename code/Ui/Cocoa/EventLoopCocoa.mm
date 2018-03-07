#import <Cocoa/Cocoa.h>

#include "Core/Log/Log.h"
#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/NSDebugAutoreleasePool.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/AllEvents.h"

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
	NSAutoreleasePool* pool = (NSAutoreleasePool*)m_pool;
	[pool release];
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
				owner->raiseEvent(&idleEvent);
				if (!idleEvent.requestedMore())
					m_idleMode = false;
			}
		}
		else
		{
			// Get application events.
			for (;;)
			{
				NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
				if (event == nil)
					break;

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
				owner->raiseEvent(&idleEvent);
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

void EventLoopCocoa::exit(int32_t exitCode)
{
	m_exitCode = exitCode;
	m_terminated = true;
}

int32_t EventLoopCocoa::getExitCode() const
{
	return m_exitCode;
}

int32_t EventLoopCocoa::getAsyncKeyState() const
{
	int32_t keyState = KsNone;
	
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

bool EventLoopCocoa::isKeyDown(VirtualKey vk) const
{
	return false;	
}

Size EventLoopCocoa::getDesktopSize() const
{
	return Size(1280, 720);
}

bool EventLoopCocoa::handleGlobalEvents(EventSubject* owner, void* event)
{
	NSEvent* evt = (NSEvent*)event;
	
	// Record modifier flags.
	m_modifierFlags = [evt modifierFlags];
	
	// Process key events; if they are globally consumed we don't dispatch further.
	NSEventType eventType = [evt type];
	if (eventType == NSKeyDown || eventType == NSKeyUp)
	{
		uint32_t systemKey = [evt keyCode];
		VirtualKey virtualKey = translateKeyCode(systemKey);
		std::wstring chs = fromNSString([evt characters]);
		
		if (eventType == NSKeyDown)
		{
			KeyDownEvent keyEvent(
				owner,
				virtualKey,
				systemKey,
				chs.empty() ? 0 : chs[0]
			);
			owner->raiseEvent(&keyEvent);
			return keyEvent.consumed();
		}
		else	// Up
		{
			KeyUpEvent keyEvent(
				owner,
				virtualKey,
				systemKey,
				chs.empty() ? 0 : chs[0]
			);
			owner->raiseEvent(&keyEvent);
			return keyEvent.consumed();
		}
	}

	return false;
}

	}
}
