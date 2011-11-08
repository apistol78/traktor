#import <Cocoa/Cocoa.h>

@interface CGLCustomWindow : NSWindow
{
	BOOL m_closed;
}

- (id) init;

- (BOOL) canBecomeKeyWindow;

- (BOOL) canBecomeMainWindow;

- (void) close;

- (BOOL) closedSinceLast;

@end
