#import <Cocoa/Cocoa.h>

#include "Core/Log/Log.h"
#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
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
	[NSApplication sharedApplication];
}

EventLoopCocoa::~EventLoopCocoa()
{
}

void EventLoopCocoa::destroy()
{
	delete this;
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
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

		if (m_idleMode)
		{
			// Poll application events.
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event != nil)
			{
				if (!handleGlobalEvents(owner, event))
				{
					if (m_modalWindows.empty() || m_modalWindows.back() == [event window])
					{
						[NSApp sendEvent: event];
						[NSApp updateWindows];
					}
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
					if (m_modalWindows.empty() || m_modalWindows.back() == [event window])
					{
						[NSApp sendEvent: event];
						[NSApp updateWindows];
					}
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
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

		if (m_idleMode)
		{
			// Poll application events.
			NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
			if (event != nil)
			{
				if (!handleGlobalEvents(owner, event))
				{
					if (m_modalWindows.empty() || m_modalWindows.back() == [event window])
					{
						[NSApp sendEvent: event];
						[NSApp updateWindows];
					}
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
					if (m_modalWindows.empty() || m_modalWindows.back() == [event window])
					{
						[NSApp sendEvent: event];
						[NSApp updateWindows];
					}
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

void EventLoopCocoa::pushModal(void* modalWindow)
{
	m_modalWindows.push_back(modalWindow);
}

void EventLoopCocoa::popModal()
{
	m_modalWindows.pop_back();
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
	else if (eventType == NSEventTypeLeftMouseDown || eventType == NSEventTypeRightMouseDown)
	{
		NSPoint mousePosition = [NSEvent mouseLocation];
		NSRect frame = [[NSScreen mainScreen] frame];

		auto pt = fromNSPoint(mousePosition);
		pt.y = frame.size.height - pt.y;

		int32_t button = 0;
		if ([evt buttonNumber] == 1)
			button = MbtLeft;
		else if ([evt buttonNumber] == 2)
			button = MbtRight;

		MouseButtonDownEvent mouseButtonDownEvent(
			owner,
			button,
			pt
		);
		owner->raiseEvent(&mouseButtonDownEvent);
	}
	else if (eventType == NSEventTypeLeftMouseUp|| eventType == NSEventTypeRightMouseUp)
	{
		NSPoint mousePosition = [NSEvent mouseLocation];
		NSRect frame = [[NSScreen mainScreen] frame];

		auto pt = fromNSPoint(mousePosition);
		pt.y = frame.size.height - pt.y;

		int32_t button = 0;
		if ([evt buttonNumber] == 1)
			button = MbtLeft;
		else if ([evt buttonNumber] == 2)
			button = MbtRight;

		MouseButtonUpEvent mouseButtonUpEvent(
			owner,
			button,
			pt
		);
		owner->raiseEvent(&mouseButtonUpEvent);
	}

	return false;
}

	}
}
