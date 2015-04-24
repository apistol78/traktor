#import <Cocoa/Cocoa.h>

#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/OsX/CGLCustomWindow.h"
#include "Render/OpenGL/Std/OsX/CGLWindow.h"
#include "Render/OpenGL/Std/OsX/CGLWindowDelegate.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
struct WindowData
{
	CGLCustomWindow* window;
	CGLWindowDelegate* delegate;
	NSMenu* menu;
	DisplayMode displayMode;
	bool fullscreen;
#if 0
	CFDictionaryRef originalMode;
#endif
	NSString* title;
};

NSString* makeNSString(const std::wstring& str)
{
	std::string mbs = wstombs(Utf8Encoding(), str);
	return [[[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding] autorelease];
}

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

void getWindowSize(NSWindow* window, int32_t& outWidth, int32_t& outHeight)
{
	NSRect frame = [window frame];
	NSRect content = [window contentRectForFrameRect: frame];

	outWidth = content.size.width;
	outHeight = content.size.height;
}

void setWindowRect(NSWindow* window, int32_t x, int32_t y, int32_t width, int32_t height)
{
	NSRect frame = [window frame];
	frame.size.width = width;
	frame.size.height = height;
	
	NSRect content = [window contentRectForFrameRect: frame];
	frame.origin.x = x;
	frame.origin.y = y;
	frame.size.width += frame.size.width - content.size.width;
	frame.size.height += frame.size.height - content.size.height;
	
	[window setFrame: frame display: YES];
}

CFArrayRef getValidDisplayModes()
{
	CFArrayRef availModes = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, NULL);
	if (!availModes)
		return 0;

	int32_t availModesCount = CFArrayGetCount(availModes);
		
	CFMutableArrayRef validModes = CFArrayCreateMutable(kCFAllocatorDefault, availModesCount, NULL);
	if (!validModes)
		return 0;

	for (int32_t i = 0; i < availModesCount; ++i)
	{
		CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(availModes, i);
		if (!mode)
			continue;
			
		CFArrayAppendValue(validModes, mode);
	}
	
	return validModes;
}

		}
	
uint32_t cglwGetDisplayModeCount()
{
	CFArrayRef displayModes = getValidDisplayModes();
	if (!displayModes)
		return 0;
		
	uint32_t displayModesCount = CFArrayGetCount(displayModes);
	CFRelease(displayModes);
	
	return displayModesCount;
}

bool cglwGetDisplayMode(uint32_t index, DisplayMode& outDisplayMode)
{
	CFArrayRef displayModes = getValidDisplayModes();
	if (!displayModes)
		return false;
	
	CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, index);
	if (!mode)
	{
		CFRelease(displayModes);
		return false;
	}
		
	outDisplayMode.width = CGDisplayModeGetWidth(mode);
	outDisplayMode.height = CGDisplayModeGetHeight(mode);
	outDisplayMode.refreshRate = CGDisplayModeGetRefreshRate(mode);
	outDisplayMode.colorBits = 32;
	
	CFRelease(displayModes);
	return true;
}

#if 0
bool cglwSetDisplayMode(const DisplayMode& displayMode)
{
	CFArrayRef displayModes = getValidDisplayModes();
	if (!displayModes)
		return false;

	uint32_t displayModesCount = CFArrayGetCount(displayModes);
	CFDictionaryRef bestMatch = 0;

	for (uint32_t i = 0; i < displayModesCount; ++i)
	{
		CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);
		if (
			CGDisplayModeGetWidth(mode) == displayMode.width &&
			CGDisplayModeGetHeight(mode) == displayMode.height &&
			CGDisplayModeGetRefreshRate(mode) == displayMode.refreshRate
		)
		{
			bestMatch = mode;
			break;
		}
	}
		
	if (!bestMatch)
	{
		log::error << L"Unable to set display mode; no such display mode supported" << Endl;
		CFRelease(displayModes);
		return false;
	}

	CGDisplaySwitchToMode(kCGDirectMainDisplay, bestMatch);
	CFRelease(displayModes);
	
	return true;
}
#endif

bool cglwGetCurrentDisplayMode(DisplayMode& outDisplayMode)
{
	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(kCGDirectMainDisplay);
	if (!mode)
		return false;
	
	outDisplayMode.width = CGDisplayModeGetWidth(mode);
	outDisplayMode.height = CGDisplayModeGetHeight(mode);
	outDisplayMode.refreshRate = CGDisplayModeGetRefreshRate(mode);
	outDisplayMode.colorBits = 32;

	return true;
}

void* cglwCreateWindow(const std::wstring& title, const DisplayMode& displayMode, bool fullscreen)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	// Change policy so menus are properly displayed.
	[NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

	[NSApplication sharedApplication];
	[NSApp finishLaunching];

	WindowData* windowData = new WindowData();
	windowData->window = 0;
	windowData->delegate = 0;
	windowData->menu = 0;
	windowData->displayMode = displayMode;
	windowData->fullscreen = fullscreen;
#if 0
	windowData->originalMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
#endif
	windowData->title = makeNSString(title);
	
	// Create window.
	if (fullscreen)
	{
		NSRect frame = [[NSScreen mainScreen] frame];

		CGDisplayCapture(kCGDirectMainDisplay);
#if 0
		if (!cglwSetDisplayMode(displayMode))
			return 0;
#endif
		NSInteger windowLevel = CGShieldingWindowLevel();

		windowData->window = [[CGLCustomWindow alloc]
			initWithContentRect: frame
			styleMask:NSBorderlessWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
			screen: [NSScreen mainScreen]
		];

		[windowData->window setLevel: windowLevel];		
		[windowData->window setHidesOnDeactivate: YES];
		[windowData->window setOpaque: YES];
	}
	else
	{
		NSRect frame = NSMakeRect(0, 0, displayMode.width, displayMode.height);
		windowData->window = [[CGLCustomWindow alloc]
			initWithContentRect: frame
			styleMask: NSTitledWindowMask | NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
			backing: NSBackingStoreBuffered
			defer: YES
		];
	}
	
	[windowData->window init];

	[windowData->window setBackgroundColor: [NSColor blackColor]];
	[windowData->window setTitle: windowData->title];
	
	NSSize minSize = { 320, 200 };
	[windowData->window setContentMinSize: minSize];
	
	if (!fullscreen)
		[windowData->window center];

	// Create our delegate in order to track if user have resized window.
	windowData->delegate = [[CGLWindowDelegate alloc] init];
	[windowData->window setDelegate: windowData->delegate];

	// Create main menu with Cmd+Q shortcut.
	windowData->menu = [[NSMenu alloc] initWithTitle: makeNSString(title)];

	id appMenuItem = [NSMenuItem new];
	[windowData->menu addItem: appMenuItem];
	[NSApp setMainMenu: windowData->menu];

	id appMenu = [[NSMenu alloc] initWithTitle: makeNSString(title)];
	id quitTitle = makeNSString(L"Quit " + title);
	id quitMenuItem = [[NSMenuItem alloc]
		initWithTitle: quitTitle
		action: nil
		keyEquivalent: @"q"
	];
	[quitMenuItem setTarget: windowData->window];
	[quitMenuItem setAction: @selector(close)];
	[appMenu addItem: quitMenuItem];
	[appMenuItem setSubmenu: appMenu];

	// Present window.
	[windowData->window makeKeyAndOrderFront: nil];
	[windowData->window makeMainWindow];

	[pool release];
	return windowData;
}

void cglwDestroyWindow(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	CGDisplayShowCursor(kCGDirectMainDisplay);

	if (windowData->fullscreen)
		CGDisplayRelease(kCGDirectMainDisplay);
	
	[windowData->window release];
	[windowData->title release];
	
	delete windowData;
}

bool cglwModifyWindow(void* windowHandle, const DisplayMode& displayMode, bool fullscreen)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	
	if (windowData->fullscreen != fullscreen)
	{
		if (fullscreen)
		{
			NSRect frame = [[NSScreen mainScreen] frame];

			CGDisplayCapture(kCGDirectMainDisplay);
#if 0
			if (!cglwSetDisplayMode(displayMode))
				return false;
#endif
			NSInteger windowLevel = CGShieldingWindowLevel();

			[windowData->window setStyleMask: NSBorderlessWindowMask];
			[windowData->window setLevel: windowLevel];
			[windowData->window setOpaque: YES];
			
			setWindowRect(
				windowData->window,
				0,
				0,
				frame.size.width,
				frame.size.height
			);
			[windowData->delegate resizedSinceLast];
		}
		else
		{
#if 0
			CGDisplaySwitchToMode(kCGDirectMainDisplay, windowData->originalMode);
#endif
			CGDisplayRelease(kCGDirectMainDisplay);
		
			[windowData->window setStyleMask: NSTitledWindowMask | NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask];
			[windowData->window setLevel: NSNormalWindowLevel];
			[windowData->window setTitle: windowData->title];
			[windowData->window setOpaque: NO];

			setWindowSize(
				windowData->window,
				displayMode.width,
				displayMode.height
			);
			[windowData->delegate resizedSinceLast];

			[windowData->window center];
		}
		
		windowData->fullscreen = fullscreen;
	}
	else
	{
		if (fullscreen)
		{
			NSRect frame = [[NSScreen mainScreen] frame];
#if 0
			if (!cglwSetDisplayMode(displayMode))
				return false;
#endif
			setWindowRect(
				windowData->window,
				0,
				0,
				frame.size.width,
				frame.size.height
			);
			[windowData->delegate resizedSinceLast];
		}
		else
		{
			setWindowSize(
				windowData->window,
				displayMode.width,
				displayMode.height
			);
			[windowData->delegate resizedSinceLast];
		}
	}

	windowData->displayMode = displayMode;
	return true;
}

bool cglwIsFullscreen(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return windowData->fullscreen;
}

bool cglwIsActive(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);

	if ([windowData->window isKeyWindow] != YES)
		return false;
	if ([windowData->window isVisible] != YES)
		return false;
		
	if ([windowData->delegate isInLiveResize] == YES)
		return false;

	return true;
}

void cglwSetCursorVisible(void* windowHandle, bool visible)
{
	if (visible)
		CGDisplayShowCursor(kCGDirectMainDisplay);
	else
		CGDisplayHideCursor(kCGDirectMainDisplay);
}

bool cglwUpdateWindow(void* windowHandle, RenderEvent& outEvent)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);

	if ([windowData->window toggledSinceLast] == YES)
	{
		outEvent.type = ReToggleFullScreen;
		return true;
	}

	if ([windowData->window closedSinceLast] == YES)
	{
		outEvent.type = ReClose;
		return true;
	}
		
	if ([windowData->delegate resizedSinceLast] == YES)
	{
		outEvent.type = ReResize;
		getWindowSize(windowData->window, outEvent.resize.width, outEvent.resize.height);
		return true;
	}

	return false;
}

void* cglwGetWindowView(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	return [windowData->window contentView];
}
	
	}
}
