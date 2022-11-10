/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <Cocoa/Cocoa.h>

@interface CGLCustomWindow : NSWindow
{
	BOOL m_closed;
	BOOL m_toggled;
}

- (id) init;

- (BOOL) canBecomeKeyWindow;

- (BOOL) canBecomeMainWindow;

- (void) close;

- (BOOL) closedSinceLast;

- (void) toggle;

- (BOOL) toggledSinceLast;

- (void) keyDown: (NSEvent*)theEvent;

- (void) keyUp: (NSEvent*)theEvent;

@end
