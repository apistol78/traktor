/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NSCustomCell_H
#define traktor_ui_NSCustomCell_H

#import <Cocoa/Cocoa.h>

@interface NSCustomCell : NSCell

- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView;

- (BOOL) continueTracking: (NSPoint)lastPoint at: (NSPoint)currentPoint inView: (NSView*)controlView;

- (void) stopTracking: (NSPoint)stopPoint inView: (NSView*)controlView mouseIsUp: (BOOL)flag;

@end

#endif	// traktor_ui_NSCustomCell_H
