#import "Render/OpenGL/Std/OsX/CGLCustomWindow.h"

#include "Core/Log/Log.h"

using namespace traktor;

@implementation CGLCustomWindow

- (id) init
{
	m_closed = NO;
	m_toggled = NO;
	return self;
}

- (BOOL) canBecomeKeyWindow
{
	return YES;
}

- (BOOL) canBecomeMainWindow
{
	return YES;
}

- (void) close
{
	m_closed = YES;
}

- (BOOL) closedSinceLast
{
	BOOL closed = m_closed;
	m_closed = NO;
	return closed;
}

- (void) toggle
{
	m_toggled = YES;
}

- (BOOL) toggledSinceLast
{
	BOOL toggled = m_toggled;
	m_toggled = NO;
	return toggled;
}

- (void) keyDown: (NSEvent*)theEvent
{
	uint32_t modifierFlags = [theEvent modifierFlags];
	int32_t keyCode = [theEvent keyCode];
	
	// Toggle fullscreen with Cmd+M or Cmd+Return key combinations.
	if (
		(modifierFlags & kCGEventFlagMaskCommand) != 0 &&
		(keyCode == 0x2e || keyCode == 0x24)
	)
	{
		[self toggle];
	}
	
	// Close application with Cmd+Q combination.
	if (
		(modifierFlags & kCGEventFlagMaskCommand) != 0 &&
		(keyCode == 0x0c)
	)
	{
		[self close];
	}
}

- (void) keyUp: (NSEvent*)theEvent
{
}

@end
