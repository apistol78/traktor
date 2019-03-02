#pragma once

#import <Cocoa/Cocoa.h>

@interface NSCustomCell : NSCell

- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView;

- (BOOL) continueTracking: (NSPoint)lastPoint at: (NSPoint)currentPoint inView: (NSView*)controlView;

- (void) stopTracking: (NSPoint)stopPoint inView: (NSView*)controlView mouseIsUp: (BOOL)flag;

@end

