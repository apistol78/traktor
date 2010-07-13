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

void setWindowSize(NSWindow* window, int32_t width, int32_t height)
{
	NSRect frame = [window frame];
	frame.size.width = width;
	frame.size.height = height;
	
	NSRect content = [window contentRectForFrameRect: frame];
	frame.size.width += frame.size.width - content.size.width;
	frame.size.height += frame.size.height - content.size.height;
	
	[window setFrame: frame display: YES];
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
	DisplayMode dm = displayMode;

	CFDictionaryRef currentMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
	CFDictionaryRef bestMatch = 0;
	
	if (dm.colorBits <= 8)
		dm.colorBits = getDictionaryLong(currentMode, kCGDisplayBitsPerPixel);
	if (dm.refreshRate == 0)
		dm.refreshRate = getDictionaryLong(currentMode, kCGDisplayRefreshRate);
	
	/*if (dm.refreshRate != 0)
	{
		bestMatch = CGDisplayBestModeForParametersAndRefreshRate(
			kCGDirectMainDisplay,
			dm.colorBits,
			dm.width,
			dm.height,
			dm.refreshRate,
			NULL
		);
	}
	else*/
	{
		bestMatch = CGDisplayBestModeForParameters(
			kCGDirectMainDisplay,
			dm.colorBits,
			dm.width,
			dm.height,
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
	
	NSRect frame = NSMakeRect(0, 0, displayMode.width, displayMode.height);
	
	if (fullscreen)
	{
		CGDisplayCapture(kCGDirectMainDisplay);

		if (!cglwSetDisplayMode(displayMode))
			return 0;
	
		NSInteger windowLevel = CGShieldingWindowLevel();

		windowData->window = [[NSWindow alloc]
			initWithContentRect: frame
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
			initWithContentRect: frame
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
	
	cglwUpdateWindow(windowData);
	
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

bool cglwModifyWindow(void* windowHandle, const DisplayMode& displayMode)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen)
	{
		if (!cglwSetDisplayMode(displayMode))
			return false;
	}
	
	setWindowSize(
		windowData->window,
		displayMode.width,
		displayMode.height
	);
	
	windowData->displayMode = displayMode;
	
	cglwUpdateWindow(windowHandle);

	return true;
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

		[windowData->window setStyleMask: NSBorderlessWindowMask];
		[windowData->window setLevel: windowLevel];
	}
	else
	{
		CGDisplaySwitchToMode(kCGDirectMainDisplay, windowData->originalMode);
		CGDisplayRelease(kCGDirectMainDisplay);
	
		[windowData->window setStyleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask];
		[windowData->window setLevel: NSNormalWindowLevel];
	}

	setWindowSize(
		windowData->window,
		windowData->displayMode.width,
		windowData->displayMode.height
	);

	[windowData->window center];
	
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
			int32_t keyCode = [event keyCode];
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
