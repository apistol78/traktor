#import <Cocoa/Cocoa.h>

#include "Core/Log/Log.h"
#include "Input/OsX/InputUtilities.h"

namespace traktor
{
	namespace input
	{

bool isInputAllowed()
{
	NSWindow* keyWindow = [NSApp keyWindow];
	if (!keyWindow)
		return false;
		
	NSPoint mouseLocationScreen = [NSEvent mouseLocation];
	NSPoint mouseLocation = [keyWindow convertScreenToBase: mouseLocationScreen];

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
	
	}
}

