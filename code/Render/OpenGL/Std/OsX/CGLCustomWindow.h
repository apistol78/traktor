#import <Cocoa/Cocoa.h>

@interface CGLCustomWindow : NSWindow
{
	BOOL m_closed;
}

- (void) init;

- (BOOL) canBecomeKeyWindow;

- (BOOL) canBecomeMainWindow;

- (void) close;

- (BOOL) closed;

@end
