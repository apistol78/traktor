#import <Cocoa/Cocoa.h>

@interface CGLCustomWindow : NSWindow
{
	BOOL m_closed;
	BOOL m_toggled;
}

- (id) init;

- (BOOL) canBecomeKeyWindow;

- (BOOL) canBecomeMainWindow;

- (void) close;

- (BOOL) closedSinceLast;

- (void) toggle;

- (BOOL) toggledSinceLast;

- (void) keyDown: (NSEvent*)theEvent;

- (void) keyUp: (NSEvent*)theEvent;

@end
