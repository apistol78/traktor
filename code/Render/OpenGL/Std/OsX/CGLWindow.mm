#import <Cocoa/Cocoa.h>

#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/OsX/CGLWindow.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getDictionaryLong(CFDictionaryRef dict, const void* key)
{
	CFNumberRef numberRef = (CFNumberRef)CFDictionaryGetValue(dict, key);
	if (!numberRef)
		return 0;
	
	long value = 0;
	CFNumberGetValue(numberRef, kCFNumberLongType, &value);
	
	return int32_t(value);
}

		}
	
uint32_t cglwGetDisplayModeCount()
{
	CFArrayRef availModes = CGDisplayAvailableModes(kCGDirectMainDisplay);
	return CFArrayGetCount(availModes);
}

bool cglwGetDisplayMode(uint32_t index, DisplayMode& outDisplayMode)
{
	CFArrayRef availModes = CGDisplayAvailableModes(kCGDirectMainDisplay);
	if (!availModes)
		return false;
	
	CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(availModes, index);
	if (!mode)
		return false;
		
	outDisplayMode.width = getDictionaryLong(mode, kCGDisplayWidth);
	outDisplayMode.height = getDictionaryLong(mode, kCGDisplayHeight);
	outDisplayMode.refreshRate = getDictionaryLong(mode, kCGDisplayRefreshRate);
	outDisplayMode.colorBits = getDictionaryLong(mode, kCGDisplayBitsPerPixel);
	
	return true;
}

bool cglwSetDisplayMode(const DisplayMode& displayMode)
{
	CFDictionaryRef bestMatch = 0;
	
	if (displayMode.refreshRate != 0)
	{
		bestMatch = CGDisplayBestModeForParametersAndRefreshRate(
			kCGDirectMainDisplay,
			displayMode.colorBits,
			displayMode.width,
			displayMode.height,
			displayMode.refreshRate,
			NULL
		);
	}
	else
	{
		bestMatch = CGDisplayBestModeForParameters(
			kCGDirectMainDisplay,
			displayMode.colorBits,
			displayMode.width,
			displayMode.height,
			NULL
		);
	}
	
	if (!bestMatch)
		return false;

	CGDisplaySwitchToMode(kCGDirectMainDisplay, bestMatch);
	return true;
}

bool cglwGetCurrentDisplayMode(DisplayMode& outDisplayMode)
{
	CFDictionaryRef mode = CGDisplayCurrentMode(kCGDirectMainDisplay);
//	if (!mode)
//		return false;
	
	outDisplayMode.width = getDictionaryLong(mode, kCGDisplayWidth);
	outDisplayMode.height = getDictionaryLong(mode, kCGDisplayHeight);
	outDisplayMode.refreshRate = getDictionaryLong(mode, kCGDisplayRefreshRate);
	outDisplayMode.colorBits = getDictionaryLong(mode, kCGDisplayBitsPerPixel);

	return true;
}

void* cglwCreateWindow(const std::wstring& title, uint32_t width, uint32_t height, bool fullscreen)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];
	[NSApp finishLaunching];

	std::string mbs = wstombs(title);
	NSString* titleStr = [[[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding] autorelease];

	NSWindow* window = 0;
	
	if (fullscreen)
	{
		CGDisplayCapture(kCGDirectMainDisplay);
		NSInteger windowLevel = CGShieldingWindowLevel();

		NSRect frame = [[NSScreen mainScreen] frame];

		window = [[NSWindow alloc]
			initWithContentRect: frame
			styleMask:NSBorderlessWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
			screen: [NSScreen mainScreen]
		];
		
		[window setLevel: windowLevel];
	}
	else
	{
		window = [[NSWindow alloc]
			initWithContentRect: NSMakeRect(50, 50, width, height)
			styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
		];
	}

	[window setBackgroundColor: [NSColor blackColor]];
	[window setAcceptsMouseMovedEvents: YES];
	[window setTitle: titleStr];
	[window center];
	
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
		NSEventType eventType = [event type];
		if (eventType != NSKeyDown && eventType != NSKeyUp)	// \hack Don't propagate key events as they cause beeps.
		{
			[NSApp sendEvent: event];
			[NSApp updateWindows];
		}
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
