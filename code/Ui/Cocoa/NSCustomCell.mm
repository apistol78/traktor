#include "Ui/Cocoa/NSCustomCell.h"

@implementation NSCustomCell

- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView
{
	NSLog(@"startTrackingAt");
	return YES;
}

- (BOOL) continueTracking: (NSPoint)lastPoint at: (NSPoint)currentPoint inView: (NSView*)controlView
{
	NSLog(@"continueTracking");
	return YES;
}

- (void) stopTracking: (NSPoint)stopPoint inView: (NSView*)controlView mouseIsUp: (BOOL)flag
{
	NSLog(@"stopTracking");
}

@end
