#import <Cocoa/Cocoa.h>

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/OsX/CGLWindow.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
struct WindowData
{
	NSWindow* window;
	DisplayMode displayMode;
	bool fullscreen;
	CFDictionaryRef originalMode;
};

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

void* cglwCreateWindow(const std::wstring& title, const DisplayMode& displayMode, bool fullscreen)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];
	[NSApp finishLaunching];

	std::string mbs = wstombs(title);
	NSString* titleStr = [[[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding] autorelease];

	WindowData* windowData = new WindowData();
	windowData->window = 0;
	windowData->displayMode = displayMode;
	windowData->fullscreen = fullscreen;
	windowData->originalMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
	
	if (fullscreen)
	{
		CGDisplayCapture(kCGDirectMainDisplay);

		if (!cglwSetDisplayMode(displayMode))
			return 0;
	
		NSInteger windowLevel = CGShieldingWindowLevel();

		windowData->window = [[NSWindow alloc]
			initWithContentRect: NSMakeRect(0, 0, displayMode.width, displayMode.height)
			styleMask:NSBorderlessWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
			screen: [NSScreen mainScreen]
		];
		
		[windowData->window setLevel: windowLevel];
	}
	else
	{
		windowData->window = [[NSWindow alloc]
			initWithContentRect: NSMakeRect(0, 0, displayMode.width, displayMode.height)
			styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
		];
	}

	[windowData->window setBackgroundColor: [NSColor blackColor]];
	[windowData->window setAcceptsMouseMovedEvents: YES];
	[windowData->window setTitle: titleStr];
	[windowData->window center];
	
	[windowData->window makeKeyAndOrderFront: nil];
	[windowData->window makeMainWindow];
	
	cglwUpdateWindow(windowData->window);
	
	[pool release];	
	
	return windowData;
}

void cglwDestroyWindow(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen)
		CGDisplayRelease(kCGDirectMainDisplay);
	
	[windowData->window release];
	
	delete windowData;
}

void cglwSetFullscreen(void* windowHandle, bool fullscreen)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen == fullscreen)
		return;

	if (fullscreen)
	{
		CGDisplayCapture(kCGDirectMainDisplay);

		if (!cglwSetDisplayMode(windowData->displayMode))
			return;

		NSInteger windowLevel = CGShieldingWindowLevel();

		[windowData->window setFrame: NSMakeRect(0, 0, windowData->displayMode.width, windowData->displayMode.height) display: YES];
		[windowData->window setStyleMask: NSBorderlessWindowMask];
		[windowData->window setLevel: windowLevel];
	}
	else
	{
		CGDisplaySwitchToMode(kCGDirectMainDisplay, windowData->originalMode);
		CGDisplayRelease(kCGDirectMainDisplay);
	
		[windowData->window setLevel: NSNormalWindowLevel];
		[windowData->window setStyleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask];
		[windowData->window center];
	}
	
	cglwUpdateWindow(windowData->window);
	
	windowData->fullscreen = fullscreen;
}

bool cglwIsFullscreen(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return windowData->fullscreen;
}

void cglwUpdateWindow(void* windowHandle)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
	if (event != nil)
	{
		NSEventType eventType = [event type];		
		if (eventType == NSKeyDown)
		{
			uint32_t keyCode = [event keyCode];
			log::debug << keyCode << Endl;
			
			if (keyCode == 122)	// F1
				cglwSetFullscreen(windowHandle, true);
			else if (keyCode == 120)	// F2
				cglwSetFullscreen(windowHandle, false);
		}
		else if (eventType != NSKeyUp)
		{
			[NSApp sendEvent: event];
			[NSApp updateWindows];
		}
	}
	[pool release];
}

void* cglwGetWindowView(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return [windowData->window contentView];
}
	
	}
}
