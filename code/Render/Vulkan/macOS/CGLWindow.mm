#import <Cocoa/Cocoa.h>

#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Vulkan/macOS/CGLCustomWindow.h"
#include "Render/Vulkan/macOS/CGLWindow.h"
#include "Render/Vulkan/macOS/CGLWindowDelegate.h"

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
	NSString* title;
};

NSString* makeNSString(const std::wstring& str)
{
	std::string mbs = wstombs(Utf8Encoding(), str);
	return [[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding];
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
	windowData->title = makeNSString(title);

	// Create window.
	if (fullscreen)
	{
		NSRect frame = [[NSScreen mainScreen] frame];

		CGDisplayCapture(kCGDirectMainDisplay);
		NSInteger windowLevel = CGShieldingWindowLevel();

		windowData->window = [[CGLCustomWindow alloc]
			initWithContentRect: frame
			styleMask:NSWindowStyleMaskBorderless
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
			styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
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
	return windowData;
}

void cglwDestroyWindow(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);

	CGDisplayShowCursor(kCGDirectMainDisplay);

	if (windowData->fullscreen)
		CGDisplayRelease(kCGDirectMainDisplay);

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
			NSInteger windowLevel = CGShieldingWindowLevel();

			[windowData->window setStyleMask: NSWindowStyleMaskBorderless];
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
			CGDisplayRelease(kCGDirectMainDisplay);

			[windowData->window setStyleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable];
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
		outEvent.type = RenderEventType::ToggleFullScreen;
		return true;
	}

	if ([windowData->window closedSinceLast] == YES)
	{
		outEvent.type = RenderEventType::Close;
		return true;
	}

	if ([windowData->delegate resizedSinceLast] == YES)
	{
		outEvent.type = RenderEventType::Resize;
		getWindowSize(windowData->window, outEvent.resize.width, outEvent.resize.height);
		return true;
	}

	return false;
}

void* cglwGetWindowView(void* windowHandle)
{
	WindowData* windowData = static_cast< WindowData* >(windowHandle);
	NSView* view = [windowData->window contentView];
	return (__bridge void*)view;
}

	}
}
