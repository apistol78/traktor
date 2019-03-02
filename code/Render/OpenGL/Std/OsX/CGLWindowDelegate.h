#import <Cocoa/Cocoa.h>

@interface CGLWindowDelegate : NSObject< NSWindowDelegate >
{
	BOOL m_resized;
	BOOL m_resizing;
}

- (id) init;

- (void) windowWillStartLiveResize: (NSNotification *)notification;

- (void) windowDidEndLiveResize: (NSNotification *)notification;

- (void) windowDidResize: (NSNotification *)notification;

- (BOOL) resizedSinceLast;

- (BOOL) isInLiveResize;

@end
