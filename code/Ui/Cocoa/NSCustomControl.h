
#import <Cocoa/Cocoa.h>

@interface NSCustomControl : NSControl
{
	id m_delegate;
}

- (void) setDelegate: (id)delegate;

- (BOOL) isFlipped;

- (void) drawRect: (NSRect)rect;

- (void) viewDidEndLiveResize;

- (void) mouseDown: (NSEvent*)theEvent;

- (void) mouseUp: (NSEvent*)theEvent;

- (void) mouseMoved: (NSEvent*)theEvent;

@end
