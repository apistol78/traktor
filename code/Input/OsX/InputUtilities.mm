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

bool getMouseRange(float& outMaxX, float& outMaxY)
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;

	NSRect frameRect = [keyWindow frame];
	NSRect contentRect = [keyWindow contentRectForFrameRect: frameRect];

	outMaxX = contentRect.size.width;
	outMaxY = contentRect.size.height;

	return true;
}

bool getMousePosition(float& outX, float& outY)
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;
    
	NSPoint mouseLocationScreen = [NSEvent mouseLocation];
	
	NSRect frameRect = [keyWindow frame];
	NSRect contentRect = [keyWindow contentRectForFrameRect: frameRect];
    
    outX = mouseLocationScreen.x - contentRect.origin.x;
    outY = contentRect.origin.y + contentRect.size.height - mouseLocationScreen.y;
    
    return true;
}

bool getMouseCenterPosition(float& outX, float& outY)
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;

	NSRect contentRect = [keyWindow contentRectForFrameRect: [keyWindow frame]];

	outX = contentRect.origin.x + contentRect.size.width / 2;
	outY = contentRect.origin.y + contentRect.size.height / 2;

	return true;
}

	}
}

