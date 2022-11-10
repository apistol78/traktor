/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import "Ui/Cocoa/NSCustomOutlineView.h"

@implementation NSCustomOutlineView

- (void) rightMouseDown: (NSEvent*)theEvent
{
	id delegate = [self delegate];

	// if ([delegate respondsToSelector:@selector(outlineViewRightMouseDown:)])
	// 	[delegate outlineViewRightMouseDown: theEvent];

	[super rightMouseDown: theEvent];
}

@end
