#import "Ui/Cocoa/NSCustomControl.h"

#include "Core/Log/Log.h"

@implementation NSCustomControl

- (void) setDelegate: (id)delegate
{
	m_delegate = delegate;
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) drawRect: (NSRect)rect
{
	if ([m_delegate respondsToSelector:@selector(drawRect:)])
		[m_delegate drawRect: rect];

	[super drawRect: rect];
}

- (void) viewDidEndLiveResize
{
	if ([m_delegate respondsToSelector:@selector(viewDidEndLiveResize)])
		[m_delegate viewDidEndLiveResize];
		
	[super viewDidEndLiveResize];
}

- (void) mouseDown: (NSEvent*)theEvent
{
	if ([m_delegate respondsToSelector:@selector(mouseDown:)])
		[m_delegate mouseDown: theEvent];

	[super mouseDown: theEvent];
}

- (void) mouseUp: (NSEvent*)theEvent
{
	if ([m_delegate respondsToSelector:@selector(mouseUp:)])
		[m_delegate mouseUp: theEvent];

	[super mouseUp: theEvent];
}

- (void) mouseMoved: (NSEvent*)theEvent
{
	if ([m_delegate respondsToSelector:@selector(mouseMoved:)])
		[m_delegate mouseMoved: theEvent];

	[super mouseMoved: theEvent];
}

@end
