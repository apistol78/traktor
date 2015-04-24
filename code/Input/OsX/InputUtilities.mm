#import <Cocoa/Cocoa.h>

#include "Core/Log/Log.h"
#include "Input/OsX/InputUtilities.h"

namespace traktor
{
	namespace input
	{

bool isInputAllowed()
{
	if (![NSApp isActive])
		return false;

	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;

	NSPoint mouseLocationScreen = [NSEvent mouseLocation];
	NSRect contentRect = [keyWindow contentRectForFrameRect: [keyWindow frame]];
	if (
		mouseLocationScreen.x < contentRect.origin.x ||
		mouseLocationScreen.y < contentRect.origin.y ||
		mouseLocationScreen.x > contentRect.origin.x + contentRect.size.width ||
		mouseLocationScreen.y > contentRect.origin.y + contentRect.size.height
	)
		return false;
	
	return true;
}

bool getMouseRange(NSSize& outRange)
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;

	NSRect frameRect = [keyWindow frame];
	NSRect contentRect = [keyWindow contentRectForFrameRect: frameRect];

	outRange = contentRect.size;
	return true;
}

bool getMousePosition(NSPoint& outMousePositionGlobal, NSPoint& outMousePositionLocal)
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;
    
	outMousePositionGlobal = [NSEvent mouseLocation];
	
	NSRect frameRect = [keyWindow frame];
	NSRect contentRect = [keyWindow contentRectForFrameRect: frameRect];
    
    outMousePositionLocal.x = outMousePositionGlobal.x - contentRect.origin.x;
    outMousePositionLocal.y = contentRect.origin.y + contentRect.size.height - outMousePositionGlobal.y;
    
    return true;
}

bool getMouseCenterPosition(NSPoint& outCenterPosition)
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;

	NSRect contentRect = [keyWindow contentRectForFrameRect: [keyWindow frame]];

	outCenterPosition.x = contentRect.origin.x + contentRect.size.width / 2;
	outCenterPosition.y = contentRect.origin.y + contentRect.size.height / 2;

	return true;
}

	}
}

