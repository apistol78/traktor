#import <Cocoa/Cocoa.h>

@interface CGLWindowDelegate : NSObject
{
	BOOL m_resized;
}

- (id) init;

- (void) windowDidResize: (NSNotification *)notification;

- (BOOL) resizedSinceLast;

@end
