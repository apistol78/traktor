#import <Cocoa/Cocoa.h>

#include "Render/OpenGL/Std/OsX/CGLWindow.h"

namespace traktor
{
	namespace render
	{

void* cglwCreateWindow(const std::wstring& title, uint32_t width, uint32_t height)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];
	[NSApp finishLaunching];

	NSWindow* window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(50, 50, width, height)
		styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
		backing: NSBackingStoreBuffered
		defer: YES
	];

	[window setAcceptsMouseMovedEvents: YES];
//	[window setTitle:makeNSString(title)];
	
	[window makeKeyAndOrderFront: nil];
	[window makeMainWindow];
	
	cglwUpdateWindow(window);
	
	[pool release];	
	
	return window;
}

void cglwDestroyWindow(void* windowHandle)
{
	NSWindow* window = (NSWindow*)windowHandle;
	[window release];
}

void cglwUpdateWindow(void* windowHandle)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
	if (event != nil)
	{
		[NSApp sendEvent: event];
		[NSApp updateWindows];
	}
	[pool release];
}

void* cglwGetWindowView(void* windowHandle)
{
	NSWindow* window = (NSWindow*)windowHandle;
	return [window contentView];
}
	
	}
}
