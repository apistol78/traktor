#ifndef traktor_ui_NSCustomCell_H
#define traktor_ui_NSCustomCell_H

#import <Cocoa/Cocoa.h>

@interface NSCustomCell : NSCell

- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView;

- (BOOL) continueTracking: (NSPoint)lastPoint at: (NSPoint)currentPoint inView: (NSView*)controlView;

- (void) stopTracking: (NSPoint)stopPoint inView: (NSView*)controlView mouseIsUp: (BOOL)flag;

@end

#endif	// traktor_ui_NSCustomCell_H
