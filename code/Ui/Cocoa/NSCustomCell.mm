/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
