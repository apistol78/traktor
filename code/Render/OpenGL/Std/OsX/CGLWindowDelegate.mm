#import "CGLWindowDelegate.h"

@implementation CGLWindowDelegate

- (id) init
{
	m_resized = NO;
}

- (void) windowDidResize: (NSNotification *)notification
{
	m_resized = YES;
}

- (BOOL) resizedSinceLast
{
	BOOL resized = m_resized;
	m_resized = NO;
	return resized;
}

@end
